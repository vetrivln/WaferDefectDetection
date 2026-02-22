#pragma once

#include <opencv2/opencv.hpp>
#include <msclr/marshal_cppstd.h>

struct Defect
{
    cv::Point2f center;
    cv::Rect    boundingBox;
    float       area;
    std::string type; // classified as either 
                      // "speck", "scratch", "cluster" for now
};

// Ensure strings returned by .NET are optimized for
// interop with std::string 
static std::string
ToStdString(System::String^ s)
{
    return msclr::interop::marshal_as<std::string>(s);
}

// Converts an OpenCV Mat to a .NET Bitmap for display in the UI.
static System::Drawing::Bitmap^
MatToBitmap(const cv::Mat& mat)
{
    cv::Mat bgr;
    if (mat.channels() == 1)
        cv::cvtColor(mat, bgr, cv::COLOR_GRAY2BGR);
    else
        bgr = mat;

    cv::Mat rgb;
	cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB); // TODO: Fix output being BGR instead of RGB

    System::Drawing::Bitmap^ bmp = gcnew System::Drawing::Bitmap(
        rgb.cols, rgb.rows,
        System::Drawing::Imaging::PixelFormat::Format24bppRgb);

    System::Drawing::Rectangle rect(0, 0, bmp->Width, bmp->Height);
    System::Drawing::Imaging::BitmapData^ bmpData =
        bmp->LockBits(rect,
            System::Drawing::Imaging::ImageLockMode::WriteOnly,
            bmp->PixelFormat);

    for (int y = 0; y < rgb.rows; y++)
        memcpy(
            (char*)bmpData->Scan0.ToPointer() + y * bmpData->Stride,
            rgb.ptr(y),
            rgb.cols * 3);

    bmp->UnlockBits(bmpData);
    return bmp;
}

// Extracts a binary mask of the lens area from the grayscale image.
static cv::Mat
extractLensMask(const cv::Mat& gray)
{
    cv::Mat mask;
    cv::threshold(gray, mask, 8, 255, cv::THRESH_BINARY);

    auto kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, { 15, 15 });
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);

    // Keep only the largest contour (the lens disc)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours,
        cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Mat cleanMask = cv::Mat::zeros(gray.size(), CV_8U);
    if (!contours.empty())
    {
        int    largest = 0;
        double maxArea = 0;
        for (int i = 0; i < (int)contours.size(); i++)
        {
            double a = cv::contourArea(contours[i]);
            if (a > maxArea)
            {
                maxArea = a;
                largest = i;
            }
        }
        cv::drawContours(cleanMask, contours, largest, 255, cv::FILLED);
    }
    return cleanMask;
}

// Corrects uneven illumination across the lens by estimating
// the background and normalizing.
static cv::Mat
correctIllumination(const cv::Mat& gray, const cv::Mat& mask, int blurSize)
{
    // blurSize must be odd
    if (blurSize % 2 == 0)
        blurSize++;

    cv::Mat floatGray;
    gray.convertTo(floatGray, CV_32F);

    cv::Mat background;
    cv::GaussianBlur(floatGray, background, { blurSize, blurSize }, 0);

    // Division is more stable than subtraction on dark images
    cv::Mat corrected;
    cv::divide(floatGray + 1.0f, background + 1.0f, corrected);
    cv::normalize(corrected, corrected, 0, 255, cv::NORM_MINMAX, CV_8U, mask);

    return corrected;
}

// Detects defects by enhancing local contrast, isolating small bright features,
static cv::Mat
detectDefects(const cv::Mat& corrected, const cv::Mat& mask, int threshold)
{
    // CLAHE — boost local contrast
    cv::Mat enhanced;
    auto clahe = cv::createCLAHE(3.0, { 8, 8 });
    clahe->apply(corrected, enhanced);

    // White top-hat — isolates small bright features
    cv::Mat tophat;
    auto kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, { 7, 7 });
    cv::morphologyEx(enhanced, tophat, cv::MORPH_TOPHAT, kernel);

    // Threshold to binary defect mask
    cv::Mat defectMask;
    cv::threshold(tophat, defectMask, threshold, 255, cv::THRESH_BINARY);

    // Remove single-pixel noise
    auto noiseKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, { 3, 3 });
    cv::morphologyEx(defectMask, defectMask, cv::MORPH_OPEN, noiseKernel);

    // Clip to lens area
    cv::bitwise_and(defectMask, mask, defectMask);

    return defectMask;
}

// Analyzes the binary defect mask to extract properties
// of each defect and classify them.
static std::vector<Defect>
analyzeDefects(const cv::Mat& defectMask)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(defectMask, contours,
        cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<Defect> defects;
    for (auto& c : contours)
    {
        float area = (float)cv::contourArea(c);
        if (area < 2.0f)
            continue;

        Defect d;
        d.area = area;
        d.boundingBox = cv::boundingRect(c);

        auto M = cv::moments(c);
        d.center = { (float)(M.m10 / M.m00), (float)(M.m01 / M.m00) };

        float w = (float)d.boundingBox.width;
        float h = (float)d.boundingBox.height;
        float ar = w / std::max<float>(h, 1.0f);

        bool isElongated = (ar > 2.5f || ar < 0.40f);
        bool isLargeEnough = (area > 20.0f);

        if (isElongated && isLargeEnough)      d.type = "scratch";
        else if (area > 150.0f)                d.type = "cluster";
        else                                   d.type = "speck";

        defects.push_back(d);
    }
    return defects;
}

// Overlaying the lens boundary and defect markers
static cv::Mat
buildAnnotatedDisplay(const cv::Mat& corrected,
    const cv::Mat& mask,
    const std::vector<Defect>& defects,
    bool                       pass,
    float                      ratio)
{
    // Base display image
    cv::Mat display;
    cv::cvtColor(corrected, display, cv::COLOR_GRAY2BGR);

    // Lens boundary
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours,
        cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::drawContours(display, contours, -1, { 0, 255, 0 }, 3);

    // Defect markers
    for (int i = 0; i < (int)defects.size(); i++)
    {
        const auto& d = defects[i];

        cv::Scalar color = (d.type == "scratch") ? cv::Scalar(0, 0, 255)
            : (d.type == "cluster") ? cv::Scalar(0, 165, 255)
            : cv::Scalar(255, 0, 255);

        int radius = std::max<float>(8, (int)std::sqrt(d.area) + 4);
        cv::circle(display, d.center, radius, color, 1);
        cv::putText(display, std::to_string(i + 1),
            { (int)d.center.x + radius + 2, (int)d.center.y + 4 },
            cv::FONT_HERSHEY_SIMPLEX, 0.4, color, 1);
    }

    return display;
}

// ─────────────────────────────────────────────────────────────
// WINFORMS UI
// ─────────────────────────────────────────────────────────────

namespace waferdefectdetection
{
    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;

    public ref class UI : public System::Windows::Forms::Form
    {
    public:
        UI(void)
        {
            InitializeComponent();
            currentDefects = gcnew System::Collections::Generic::List<IntPtr>();
            hasImage = false;
        }

    protected:
        ~UI()
        {
            if (components)
                delete components;
        }

        // ── Controls ────────────────────────────────────────────────
    private:
        System::Windows::Forms::OpenFileDialog^ dlg;
        System::Windows::Forms::Button^ btnLoad;
        System::Windows::Forms::Button^ btnAnalyze;
        System::Windows::Forms::Label^ lblFilename;
        System::Windows::Forms::Label^ lblVerdict;
        System::Windows::Forms::Label^ lblDefectInfo;
        System::Windows::Forms::Label^ lblThreshold;
        System::Windows::Forms::Label^ lblBlur;
        System::Windows::Forms::PictureBox^ pbOriginal;
        System::Windows::Forms::PictureBox^ pbAnalyzed;
        System::Windows::Forms::PictureBox^ pbZoom;
        System::Windows::Forms::NumericUpDown^ nudThreshold;
        System::Windows::Forms::NumericUpDown^ nudBlur;
        System::ComponentModel::Container^ components;

    private:
        bool                 hasImage;
        cv::Mat*             storedGray;
        cv::Mat*             storedCorrected;
        cv::Mat*             storedMask;
        cv::Mat*             storedDisplay;
        std::vector<Defect>* storedDefects;
    private: 
        System::Windows::Forms::Label^ gBlurThreshold;
        System::Collections::Generic::List<IntPtr>^ currentDefects;

#pragma region Windows Form Designer generated code
        void InitializeComponent(void)
        {
            this->dlg = (gcnew System::Windows::Forms::OpenFileDialog());
            this->btnLoad = (gcnew System::Windows::Forms::Button());
            this->btnAnalyze = (gcnew System::Windows::Forms::Button());
            this->lblFilename = (gcnew System::Windows::Forms::Label());
            this->lblVerdict = (gcnew System::Windows::Forms::Label());
            this->lblDefectInfo = (gcnew System::Windows::Forms::Label());
            this->lblThreshold = (gcnew System::Windows::Forms::Label());
            this->lblBlur = (gcnew System::Windows::Forms::Label());
            this->pbOriginal = (gcnew System::Windows::Forms::PictureBox());
            this->pbAnalyzed = (gcnew System::Windows::Forms::PictureBox());
            this->pbZoom = (gcnew System::Windows::Forms::PictureBox());
            this->nudThreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->nudBlur = (gcnew System::Windows::Forms::NumericUpDown());
            this->gBlurThreshold = (gcnew System::Windows::Forms::Label());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pbOriginal))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pbAnalyzed))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pbZoom))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudThreshold))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudBlur))->BeginInit();
            this->SuspendLayout();
            // 
            // btnLoad
            // 
            this->btnLoad->Location = System::Drawing::Point(12, 12);
            this->btnLoad->Name = L"btnLoad";
            this->btnLoad->Size = System::Drawing::Size(140, 45);
            this->btnLoad->TabIndex = 0;
            this->btnLoad->Text = L"Load Image";
            this->btnLoad->Click += gcnew System::EventHandler(this, &UI::btnLoad_Click);
            // 
            // btnAnalyze
            // 
            this->btnAnalyze->Enabled = false;
            this->btnAnalyze->Location = System::Drawing::Point(12, 65);
            this->btnAnalyze->Name = L"btnAnalyze";
            this->btnAnalyze->Size = System::Drawing::Size(140, 45);
            this->btnAnalyze->TabIndex = 1;
            this->btnAnalyze->Text = L"Identify Defects";
            this->btnAnalyze->Click += gcnew System::EventHandler(this, &UI::btnAnalyze_Click);
            // 
            // lblFilename
            // 
            this->lblFilename->Location = System::Drawing::Point(165, 25);
            this->lblFilename->Name = L"lblFilename";
            this->lblFilename->Size = System::Drawing::Size(400, 20);
            this->lblFilename->TabIndex = 2;
            this->lblFilename->Text = L"No file selected";
            // 
            // lblVerdict
            // 
            this->lblVerdict->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, System::Drawing::FontStyle::Bold));
            this->lblVerdict->Location = System::Drawing::Point(165, 55);
            this->lblVerdict->Name = L"lblVerdict";
            this->lblVerdict->Size = System::Drawing::Size(600, 28);
            this->lblVerdict->TabIndex = 3;
            // 
            // lblDefectInfo
            // 
            this->lblDefectInfo->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
            this->lblDefectInfo->Location = System::Drawing::Point(1153, 390);
            this->lblDefectInfo->Name = L"lblDefectInfo";
            this->lblDefectInfo->Size = System::Drawing::Size(320, 120);
            this->lblDefectInfo->TabIndex = 11;
            this->lblDefectInfo->Text = L"Click a defect on the right\nimage to inspect it here";
            // 
            // lblThreshold
            // 
            this->lblThreshold->Location = System::Drawing::Point(620, 18);
            this->lblThreshold->Name = L"lblThreshold";
            this->lblThreshold->Size = System::Drawing::Size(180, 20);
            this->lblThreshold->TabIndex = 4;
            this->lblThreshold->Text = L"Detection Threshold:";
            // 
            // lblBlur
            // 
            this->lblBlur->Location = System::Drawing::Point(620, 55);
            this->lblBlur->Name = L"lblBlur";
            this->lblBlur->Size = System::Drawing::Size(180, 20);
            this->lblBlur->TabIndex = 6;
            this->lblBlur->Text = L"Gaussian Blur Size:";
            // 
            // pbOriginal
            // 
            this->pbOriginal->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->pbOriginal->Location = System::Drawing::Point(27, 121);
            this->pbOriginal->Name = L"pbOriginal";
            this->pbOriginal->Size = System::Drawing::Size(500, 483);
            this->pbOriginal->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
            this->pbOriginal->TabIndex = 8;
            this->pbOriginal->TabStop = false;
            // 
            // pbAnalyzed
            // 
            this->pbAnalyzed->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->pbAnalyzed->Cursor = System::Windows::Forms::Cursors::Cross;
            this->pbAnalyzed->Location = System::Drawing::Point(599, 121);
            this->pbAnalyzed->Name = L"pbAnalyzed";
            this->pbAnalyzed->Size = System::Drawing::Size(475, 483);
            this->pbAnalyzed->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
            this->pbAnalyzed->TabIndex = 9;
            this->pbAnalyzed->TabStop = false;
            this->pbAnalyzed->Click += gcnew System::EventHandler(this, &UI::pbAnalyzed_Click);
            // 
            // pbZoom
            // 
            this->pbZoom->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->pbZoom->Location = System::Drawing::Point(1153, 43);
            this->pbZoom->Name = L"pbZoom";
            this->pbZoom->Size = System::Drawing::Size(320, 320);
            this->pbZoom->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
            this->pbZoom->TabIndex = 10;
            this->pbZoom->TabStop = false;
            // 
            // nudThreshold
            // 
            this->nudThreshold->Location = System::Drawing::Point(810, 15);
            this->nudThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->nudThreshold->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->nudThreshold->Name = L"nudThreshold";
            this->nudThreshold->Size = System::Drawing::Size(80, 22);
            this->nudThreshold->TabIndex = 5;
            this->nudThreshold->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 17, 0, 0, 0 });
            // 
            // nudBlur
            // 
            this->nudBlur->Location = System::Drawing::Point(810, 52);
            this->nudBlur->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 401, 0, 0, 0 });
            this->nudBlur->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 75, 0, 0, 0 });
            this->nudBlur->Name = L"nudBlur";
            this->nudBlur->Size = System::Drawing::Size(80, 22);
            this->nudBlur->TabIndex = 7;
            this->nudBlur->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 201, 0, 0, 0 });
            // 
            // gBlurThreshold
            // 
            this->gBlurThreshold->Location = System::Drawing::Point(620, 55);
            this->gBlurThreshold->Name = L"gBlurThreshold";
            this->gBlurThreshold->Size = System::Drawing::Size(180, 20);
            this->gBlurThreshold->TabIndex = 12;
            this->gBlurThreshold->Text = L"Gaussian Blur Threshold:";
            // 
            // UI
            // 
            this->ClientSize = System::Drawing::Size(1528, 673);
            this->Controls->Add(this->gBlurThreshold);
            this->Controls->Add(this->btnLoad);
            this->Controls->Add(this->btnAnalyze);
            this->Controls->Add(this->lblFilename);
            this->Controls->Add(this->lblVerdict);
            this->Controls->Add(this->lblThreshold);
            this->Controls->Add(this->nudThreshold);
            this->Controls->Add(this->lblBlur);
            this->Controls->Add(this->nudBlur);
            this->Controls->Add(this->pbOriginal);
            this->Controls->Add(this->pbAnalyzed);
            this->Controls->Add(this->pbZoom);
            this->Controls->Add(this->lblDefectInfo);
            this->Name = L"UI";
            this->Text = L"Wafer Defect Inspector";
            this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pbOriginal))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pbAnalyzed))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pbZoom))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudThreshold))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->nudBlur))->EndInit();
            this->ResumeLayout(false);

        }
#pragma endregion

    private: System::Void
        btnLoad_Click(System::Object^ sender, System::EventArgs^ e)
    {
        dlg->Filter = "BMP Images|*.bmp|All Files|*.*";
        if (dlg->ShowDialog() != System::Windows::Forms::DialogResult::OK)
            return;

        std::string path = ToStdString(dlg->FileName);

        cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
        if (img.empty())
        {
            MessageBox::Show("Failed to load image.");
            return;
        }

        storedGray = new cv::Mat();
        storedMask = new cv::Mat();
        cv::cvtColor(img, *storedGray, cv::COLOR_BGR2GRAY);
        *storedMask = extractLensMask(*storedGray);

        pbOriginal->Image = MatToBitmap(img);
        pbAnalyzed->Image = nullptr;
        pbZoom->Image = nullptr;
        lblVerdict->Text = "";
        lblDefectInfo->Text =
            L"Click a defect on the right\nimage to inspect it here";
        lblFilename->Text = System::IO::Path::GetFileName(dlg->FileName);

        hasImage = true;
        btnAnalyze->Enabled = true;
    }

    private: System::Void
        btnAnalyze_Click(System::Object^ sender, System::EventArgs^ e)
    {
        if (!hasImage)
            return;

        int blurSize = static_cast<int> (nudBlur->Value);
        int threshold = static_cast<int> (nudThreshold->Value);

        storedCorrected = new cv::Mat(
            correctIllumination(*storedGray, *storedMask, blurSize));

        cv::Mat defectMask = detectDefects(*storedCorrected, *storedMask, threshold);

        storedDefects = new std::vector<Defect>(analyzeDefects(defectMask));

        // PASS / FAIL
        float lensPixels = (float)cv::countNonZero(*storedMask);
        float defectPixels = (float)cv::countNonZero(defectMask);
        float ratio = defectPixels / std::max<float>(lensPixels, 1.0f);
        bool  pass = (ratio < 0.000005f);

        storedDisplay = new cv::Mat(
            buildAnnotatedDisplay(*storedCorrected, *storedMask,
                *storedDefects, pass, ratio));

        pbAnalyzed->Image = MatToBitmap(*storedDisplay);

        lblVerdict->Text = System::String::Format(
            "{0}  |  Defects: {1}  |  Area: {2:F4}%",
            pass ? "PASS" : "FAIL",
            storedDefects->size(),
            ratio * 100.0f);

        lblVerdict->ForeColor = pass
            ? System::Drawing::Color::Green
            : System::Drawing::Color::Red;
    }

           // ── CLICK TO ZOOM ────────────────────────────────────────────
    private: System::Void
        pbAnalyzed_Click(System::Object^ sender, System::EventArgs^ e)
    {
        if (!hasImage || !storedDefects || storedDefects->empty())
            return;

        auto me = safe_cast<System::Windows::Forms::MouseEventArgs^> (e);

        // Convert PictureBox click coords → image coords
        // Account for Zoom mode letterboxing
        int   imgW = storedDisplay->cols;
        int   imgH = storedDisplay->rows;
        int   boxW = pbAnalyzed->Width;
        int   boxH = pbAnalyzed->Height;
        float scale = std::min<float>((float)boxW / imgW, (float)boxH / imgH);
        int   offsetX = (boxW - (int)(imgW * scale)) / 2;
        int   offsetY = (boxH - (int)(imgH * scale)) / 2;

        float imgX = (me->X - offsetX) / scale;
        float imgY = (me->Y - offsetY) / scale;

        // Subtract banner height
        imgY -= 70;

        // Find nearest defect
        int   nearestIdx = 0;
        float nearestDist = FLT_MAX;
        for (int i = 0; i < (int)storedDefects->size(); i++)
        {
            float dx = (*storedDefects)[i].center.x - imgX;
            float dy = (*storedDefects)[i].center.y - imgY;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < nearestDist)
            {
                nearestDist = dist;
                nearestIdx = i;
            }
        }

        const Defect& d = (*storedDefects)[nearestIdx];

        // Crop around defect from corrected image (clean, no overlays)
        int padding = 50;
        int x = std::max<float>(0, (int)d.center.x - padding);
        int y = std::max<float>(0, (int)d.center.y - padding);
        int w = std::min<float>(storedCorrected->cols - x, padding * 2);
        int h = std::min<float>(storedCorrected->rows - y, padding * 2);

        cv::Mat crop = (*storedCorrected)(cv::Rect(x, y, w, h)).clone();

        // Upscale
        cv::Mat zoomed;
        cv::resize(crop, zoomed, { 320, 320 }, 0, 0, cv::INTER_NEAREST);

        // Crosshair at defect center
        cv::Mat zoomedColor;
        cv::cvtColor(zoomed, zoomedColor, cv::COLOR_GRAY2BGR);
        int cx = (int)((d.center.x - x) * (320.0f / w));
        int cy = (int)((d.center.y - y) * (320.0f / h));
        cv::drawMarker(zoomedColor, { cx, cy },
            { 0, 255, 255 }, cv::MARKER_CROSS, 20, 1);

        pbZoom->Image = MatToBitmap(zoomedColor);

        lblDefectInfo->Text = System::String::Format(
            "Defect #{0}\nType:      {1}\nArea:      {2:F1} px\nLocation: ({3:F0}, {4:F0})",
            nearestIdx + 1,
            gcnew System::String(d.type.c_str()),
            d.area,
            d.center.x, d.center.y);
    }
    };
}