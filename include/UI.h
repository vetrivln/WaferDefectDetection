#pragma once

#include <opencv2/opencv.hpp>
#include <msclr/marshal_cppstd.h>
#include "defect_processing.h"
#include "defect_utils.h"

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
    UI (void)
    {
      InitializeComponent ();
      current_defects_ = gcnew System::Collections::Generic::List<IntPtr> ();
      has_image_ = false;
    }

  protected:
    ~UI (void)
    {
      if (components_)
        delete components_;
    }

  private:
    /* Controls */
    System::Windows::Forms::OpenFileDialog^ dlg_;
    System::Windows::Forms::Button^ btn_load_;
    System::Windows::Forms::Button^ btn_analyze_;
    System::Windows::Forms::Label^ lbl_filename_;
    System::Windows::Forms::Label^ lbl_verdict_;
    System::Windows::Forms::Label^ lbl_defect_info_;
    System::Windows::Forms::Label^ lbl_threshold_;
    System::Windows::Forms::PictureBox^ pb_original_;
    System::Windows::Forms::PictureBox^ pb_analyzed_;
    System::Windows::Forms::PictureBox^ pb_zoom_;
    System::Windows::Forms::NumericUpDown^ nud_threshold_;
    System::Windows::Forms::NumericUpDown^ nud_blur_;
    System::Windows::Forms::Label^ lbl_gaussian_blur_;
    System::Windows::Forms::FlowLayoutPanel^ flp_defects_;
    System::Windows::Forms::Label^ lbl_defect_list_title_;
    System::ComponentModel::Container^ components_;

    /* State */
    bool has_image_;
    cv::Mat* stored_gray_;
    cv::Mat* stored_corrected_;
    cv::Mat* stored_mask_;
    cv::Mat* stored_display_;
    std::vector<Defect>* stored_defects_;
    System::Collections::Generic::List<IntPtr>^ current_defects_;

#pragma region Windows Form Designer generated code
    void
    InitializeComponent (void)
    {
      this->dlg_ = (gcnew System::Windows::Forms::OpenFileDialog ());
      this->btn_load_ = (gcnew System::Windows::Forms::Button ());
      this->btn_analyze_ = (gcnew System::Windows::Forms::Button ());
      this->lbl_filename_ = (gcnew System::Windows::Forms::Label ());
      this->lbl_verdict_ = (gcnew System::Windows::Forms::Label ());
      this->lbl_defect_info_ = (gcnew System::Windows::Forms::Label ());
      this->lbl_threshold_ = (gcnew System::Windows::Forms::Label ());
      this->pb_original_ = (gcnew System::Windows::Forms::PictureBox ());
      this->pb_analyzed_ = (gcnew System::Windows::Forms::PictureBox ());
      this->pb_zoom_ = (gcnew System::Windows::Forms::PictureBox ());
      this->nud_threshold_ = (gcnew System::Windows::Forms::NumericUpDown ());
      this->nud_blur_ = (gcnew System::Windows::Forms::NumericUpDown ());
      this->lbl_gaussian_blur_ = (gcnew System::Windows::Forms::Label ());
      this->flp_defects_ = (gcnew System::Windows::Forms::FlowLayoutPanel ());
      this->lbl_defect_list_title_ = (gcnew System::Windows::Forms::Label ());

      (cli::safe_cast<System::ComponentModel::ISupportInitialize^> (this->pb_original_))->BeginInit ();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^> (this->pb_analyzed_))->BeginInit ();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^> (this->pb_zoom_))->BeginInit ();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^> (this->nud_threshold_))->BeginInit ();
      (cli::safe_cast<System::ComponentModel::ISupportInitialize^> (this->nud_blur_))->BeginInit ();
      this->SuspendLayout ();

      // 
      // btn_load_
      // 
      this->btn_load_->Location = System::Drawing::Point(12, 12);
      this->btn_load_->Name = L"btn_load_";
      this->btn_load_->Size = System::Drawing::Size(140, 45);
      this->btn_load_->TabIndex = 0;
      this->btn_load_->Text = L"Load Image";
      this->btn_load_->Click += gcnew System::EventHandler(this, &UI::btn_load_click);
      // 
      // btn_analyze_
      // 
      this->btn_analyze_->Enabled = false;
      this->btn_analyze_->Location = System::Drawing::Point(12, 65);
      this->btn_analyze_->Name = L"btn_analyze_";
      this->btn_analyze_->Size = System::Drawing::Size(140, 45);
      this->btn_analyze_->TabIndex = 1;
      this->btn_analyze_->Text = L"Identify Defects";
      this->btn_analyze_->Click += gcnew System::EventHandler(this, &UI::btn_analyze_click);
      // 
      // lbl_filename_
      // 
      this->lbl_filename_->Location = System::Drawing::Point(21, 124);
      this->lbl_filename_->Name = L"lbl_filename_";
      this->lbl_filename_->Size = System::Drawing::Size(400, 20);
      this->lbl_filename_->TabIndex = 2;
      this->lbl_filename_->Text = L"No file selected";
      // 
      // lbl_verdict_
      // 
      this->lbl_verdict_->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 14, System::Drawing::FontStyle::Bold));
      this->lbl_verdict_->Location = System::Drawing::Point(208, 7);
      this->lbl_verdict_->Name = L"lbl_verdict_";
      this->lbl_verdict_->Size = System::Drawing::Size(600, 28);
      this->lbl_verdict_->TabIndex = 3;
      // 
      // lbl_defect_info_
      // 
      this->lbl_defect_info_->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10));
      this->lbl_defect_info_->Location = System::Drawing::Point(552, 565);
      this->lbl_defect_info_->Name = L"lbl_defect_info_";
      this->lbl_defect_info_->Size = System::Drawing::Size(320, 47);
      this->lbl_defect_info_->TabIndex = 11;
      this->lbl_defect_info_->Text = L"Click a defect to inspect it here";
      // 
      // lbl_threshold_
      // 
      this->lbl_threshold_->Location = System::Drawing::Point(187, 55);
      this->lbl_threshold_->Name = L"lbl_threshold_";
      this->lbl_threshold_->Size = System::Drawing::Size(180, 20);
      this->lbl_threshold_->TabIndex = 4;
      this->lbl_threshold_->Text = L"Detection Threshold:";
      // 
      // pb_original_
      // 
      this->pb_original_->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
      this->pb_original_->Location = System::Drawing::Point(556, 238);
      this->pb_original_->Name = L"pb_original_";
      this->pb_original_->Size = System::Drawing::Size(310, 310);
      this->pb_original_->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
      this->pb_original_->TabIndex = 8;
      this->pb_original_->TabStop = false;
      // 
      // pb_analyzed_
      // 
      this->pb_analyzed_->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
      this->pb_analyzed_->Cursor = System::Windows::Forms::Cursors::Cross;
      this->pb_analyzed_->Location = System::Drawing::Point(24, 150);
      this->pb_analyzed_->Name = L"pb_analyzed_";
      this->pb_analyzed_->Size = System::Drawing::Size(500, 500);
      this->pb_analyzed_->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
      this->pb_analyzed_->TabIndex = 9;
      this->pb_analyzed_->TabStop = false;
      this->pb_analyzed_->Click += gcnew System::EventHandler(this, &UI::pb_analyzed_click);
      // 
      // pb_zoom_
      // 
      this->pb_zoom_->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
      this->pb_zoom_->Location = System::Drawing::Point(661, 55);
      this->pb_zoom_->Name = L"pb_zoom_";
      this->pb_zoom_->Size = System::Drawing::Size(192, 168);
      this->pb_zoom_->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
      this->pb_zoom_->TabIndex = 10;
      this->pb_zoom_->TabStop = false;
      // 
      // nud_threshold_
      // 
      this->nud_threshold_->Location = System::Drawing::Point(400, 53);
      this->nud_threshold_->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
      this->nud_threshold_->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
      this->nud_threshold_->Name = L"nud_threshold_";
      this->nud_threshold_->Size = System::Drawing::Size(80, 22);
      this->nud_threshold_->TabIndex = 5;
      this->nud_threshold_->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 17, 0, 0, 0 });
      // 
      // nud_blur_
      // 
      this->nud_blur_->Location = System::Drawing::Point(400, 88);
      this->nud_blur_->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 401, 0, 0, 0 });
      this->nud_blur_->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 75, 0, 0, 0 });
      this->nud_blur_->Name = L"nud_blur_";
      this->nud_blur_->Size = System::Drawing::Size(80, 22);
      this->nud_blur_->TabIndex = 7;
      this->nud_blur_->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 201, 0, 0, 0 });
      // 
      // lbl_gaussian_blur_
      // 
      this->lbl_gaussian_blur_->Location = System::Drawing::Point(187, 90);
      this->lbl_gaussian_blur_->Name = L"lbl_gaussian_blur_";
      this->lbl_gaussian_blur_->Size = System::Drawing::Size(180, 20);
      this->lbl_gaussian_blur_->TabIndex = 12;
      this->lbl_gaussian_blur_->Text = L"Gaussian Blur Threshold:";
      // 
      // flp_defects_
      // 
      this->flp_defects_->AutoScroll = true;
      this->flp_defects_->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(30)), static_cast<System::Int32>(static_cast<System::Byte>(30)),
          static_cast<System::Int32>(static_cast<System::Byte>(30)));
      this->flp_defects_->FlowDirection = System::Windows::Forms::FlowDirection::TopDown;
      this->flp_defects_->Location = System::Drawing::Point(878, 32);
      this->flp_defects_->Name = L"flp_defects_";
      this->flp_defects_->Padding = System::Windows::Forms::Padding(4);
      this->flp_defects_->Size = System::Drawing::Size(364, 618);
      this->flp_defects_->TabIndex = 0;
      this->flp_defects_->WrapContents = false;
      // 
      // lbl_defect_list_title_
      // 
      this->lbl_defect_list_title_->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Bold));
      this->lbl_defect_list_title_->Location = System::Drawing::Point(874, 7);
      this->lbl_defect_list_title_->Name = L"lbl_defect_list_title_";
      this->lbl_defect_list_title_->Size = System::Drawing::Size(340, 22);
      this->lbl_defect_list_title_->TabIndex = 13;
      this->lbl_defect_list_title_->Text = L"Defect List";
      // 
      // UI
      // 
      this->AutoScroll = true;
      this->AutoSize = true;
      this->AutoSizeMode = System::Windows::Forms::AutoSizeMode::GrowAndShrink;
      this->ClientSize = System::Drawing::Size(1262, 673);
      this->Controls->Add(this->flp_defects_);
      this->Controls->Add(this->lbl_gaussian_blur_);
      this->Controls->Add(this->btn_load_);
      this->Controls->Add(this->btn_analyze_);
      this->Controls->Add(this->lbl_filename_);
      this->Controls->Add(this->lbl_verdict_);
      this->Controls->Add(this->lbl_threshold_);
      this->Controls->Add(this->nud_threshold_);
      this->Controls->Add(this->nud_blur_);
      this->Controls->Add(this->pb_original_);
      this->Controls->Add(this->pb_analyzed_);
      this->Controls->Add(this->pb_zoom_);
      this->Controls->Add(this->lbl_defect_info_);
      this->Controls->Add(this->lbl_defect_list_title_);
      this->Name = L"UI";
      this->Text = L"Wafer Defect Inspector";
      this->WindowState = System::Windows::Forms::FormWindowState::Maximized;

      this->ResumeLayout (false);
    }
#pragma endregion

  private:
    void
    select_defect (int idx)
    {
      const Defect& d = (*stored_defects_)[idx];

      int padding = 50;
      int x = std::max<float> (0, (int) d.center.x - padding);
      int y = std::max<float> (0, (int) d.center.y - padding);
      int w = std::min<float> (stored_corrected_->cols - x, padding * 2);
      int h = std::min<float> (stored_corrected_->rows - y, padding * 2);

      cv::Mat crop = (*stored_corrected_) (cv::Rect (x, y, w, h)).clone ();
      cv::Mat zoomed;
      cv::resize (crop, zoomed, {320, 320}, 0, 0, cv::INTER_NEAREST);

      cv::Mat zoomed_color;
      cv::cvtColor (zoomed, zoomed_color, cv::COLOR_GRAY2BGR);

      int cx = (int) ((d.center.x - x) * (320.0f / w));
      int cy = (int) ((d.center.y - y) * (320.0f / h));
      cv::drawMarker (zoomed_color, {cx, cy},
                      {0, 255, 255}, cv::MARKER_CROSS, 20, 1);

      pb_zoom_->Image = mat_to_bitmap (zoomed_color);

      lbl_defect_info_->Text = System::String::Format (
        "Defect #{0}\nType:      {1}\nArea:      {2:F1} px\nAR:      {5:F1} px\nLocation: ({3:F0}, {4:F0})",
        idx + 1,
        gcnew System::String (d.type.c_str ()),
        d.area,
        d.center.x,
        d.center.y,
        d.ar);
    }

    void
    populate_defect_list (void)
    {
      flp_defects_->Controls->Clear ();

      for (int i = 0; i < (int) stored_defects_->size (); i++)
        {
          const Defect& d = (*stored_defects_)[i];

          System::Windows::Forms::Panel^ card = gcnew System::Windows::Forms::Panel ();
          card->Size = System::Drawing::Size (310, 76);
          card->Margin = System::Windows::Forms::Padding (4, 4, 4, 0);
          card->BackColor = System::Drawing::Color::FromArgb (50, 50, 55);
          card->Cursor = System::Windows::Forms::Cursors::Hand;
          card->Tag = i;

          int pad = 30;
          int tx = std::max<float> (0, (int) d.center.x - pad);
          int ty = std::max<float> (0, (int) d.center.y - pad);
          int tw = std::min<float> (stored_corrected_->cols - tx, pad * 2);
          int th = std::min<float> (stored_corrected_->rows - ty, pad * 2);

          cv::Mat thumb = (*stored_corrected_) (cv::Rect (tx, ty, tw, th)).clone ();
          cv::Mat thumb_small;
          cv::resize (thumb, thumb_small, {64, 64}, 0, 0, cv::INTER_NEAREST);

          System::Windows::Forms::PictureBox^ pb = gcnew System::Windows::Forms::PictureBox ();
          pb->Image = mat_to_bitmap (thumb_small);
          pb->Size = System::Drawing::Size (64, 64);
          pb->Location = System::Drawing::Point (4, 6);
          pb->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
          pb->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
          pb->Tag = i;

          System::Drawing::Color type_color;

          if (d.type == "scratch")
            type_color = System::Drawing::Color::FromArgb (255, 80, 80);
          else if (d.type == "cluster")
            type_color = System::Drawing::Color::FromArgb (255, 165, 0);
          else
            type_color = System::Drawing::Color::FromArgb (220, 80, 220);

          System::String^ type_str = gcnew System::String (d.type.c_str ());

          System::Windows::Forms::Label^ lbl = gcnew System::Windows::Forms::Label ();
          lbl->Text = System::String::Format (
            "#{0}  {1}\nArea: {2:F1} px\nAR: {5:F1}\n({3:F0}, {4:F0})",
            i + 1, type_str, d.area, d.center.x, d.center.y, d.ar);
          lbl->ForeColor = type_color;
          lbl->Font = gcnew System::Drawing::Font (L"Consolas", 9);
          lbl->Location = System::Drawing::Point (74, 6);
          lbl->Size = System::Drawing::Size (228, 64);
          lbl->Tag = i;

          card->Click += gcnew System::EventHandler (this, &UI::defect_card_click);
          pb->Click += gcnew System::EventHandler (this, &UI::defect_card_click);
          lbl->Click += gcnew System::EventHandler (this, &UI::defect_card_click);

          card->Controls->Add (pb);
          card->Controls->Add (lbl);
          flp_defects_->Controls->Add (card);
        }
    }

    System::Void
    defect_card_click (System::Object^ sender, System::EventArgs^ e)
    {
      System::Windows::Forms::Control^ ctrl = safe_cast<System::Windows::Forms::Control^> (sender);
      int idx = safe_cast<int> (ctrl->Tag);
      select_defect (idx);
    }

    System::Void
    btn_load_click (System::Object^ sender, System::EventArgs^ e)
    {
      dlg_->Filter = "BMP Images|*.bmp|All Files|*.*";

      if (dlg_->ShowDialog () != System::Windows::Forms::DialogResult::OK)
        return;

      std::string path = to_std_string (dlg_->FileName);

      cv::Mat img = cv::imread (path, cv::IMREAD_COLOR);

      if (img.empty ())
        {
          MessageBox::Show ("Failed to load image.");
          return;
        }

      stored_gray_ = new cv::Mat ();
      stored_mask_ = new cv::Mat ();

      cv::cvtColor (img, *stored_gray_, cv::COLOR_BGR2GRAY);
      *stored_mask_ = extract_lens_mask (*stored_gray_);

      pb_original_->Image = Image::FromFile (dlg_->FileName);
      pb_analyzed_->Image = nullptr;
      pb_zoom_->Image = nullptr;
      lbl_verdict_->Text = "";
      lbl_defect_info_->Text = L"Click a defect on the right\nimage to inspect it here";
      lbl_filename_->Text = System::IO::Path::GetFileName (dlg_->FileName);
      flp_defects_->Controls->Clear ();

      has_image_ = true;
      btn_analyze_->Enabled = true;
    }

    System::Void
    btn_analyze_click (System::Object^ sender, System::EventArgs^ e)
    {
      if (!has_image_)
        return;

      int blur_size = static_cast<int> (nud_blur_->Value);
      int threshold = static_cast<int> (nud_threshold_->Value);

      stored_corrected_ = new cv::Mat (
        correct_illumination (*stored_gray_, *stored_mask_, blur_size));

      cv::Mat defect_mask = detect_defects (*stored_corrected_, *stored_mask_, threshold);

      stored_defects_ = new std::vector<Defect> (analyze_defects (defect_mask));

      float lens_pixels = (float) cv::countNonZero (*stored_mask_);
      float defect_pixels = (float) cv::countNonZero (defect_mask);
      float ratio = defect_pixels / std::max<float> (lens_pixels, 1.0f);
      bool pass = (ratio < 0.000005f);

      stored_display_ = new cv::Mat (
        build_annotated_display (*stored_corrected_, *stored_mask_, *stored_defects_, pass, ratio));

      pb_analyzed_->Image = mat_to_bitmap (*stored_display_);

      lbl_verdict_->Text = System::String::Format (
        "{0}  |  Defects: {1}  |  Area: {2:F4}%",
        pass ? "Y" : "N",
        stored_defects_->size (),
        ratio * 100.0f);

      lbl_verdict_->ForeColor = pass ? System::Drawing::Color::Green
                                     : System::Drawing::Color::Red;

      populate_defect_list ();
    }

    System::Void
    pb_analyzed_click (System::Object^ sender, System::EventArgs^ e)
    {
      if (!has_image_ || !stored_defects_ || stored_defects_->empty ())
        return;

      auto me = safe_cast<System::Windows::Forms::MouseEventArgs^> (e);

      int img_w = stored_display_->cols;
      int img_h = stored_display_->rows;
      int box_w = pb_analyzed_->Width;
      int box_h = pb_analyzed_->Height;
      float scale = std::min<float> ((float) box_w / img_w, (float) box_h / img_h);
      int off_x = (box_w - (int) (img_w * scale)) / 2;
      int off_y = (box_h - (int) (img_h * scale)) / 2;

      float img_x = (me->X - off_x) / scale;
      float img_y = (me->Y - off_y) / scale - 70;

      int nearest_idx = 0;
      float nearest_dist = FLT_MAX;

      for (int i = 0; i < (int) stored_defects_->size (); i++)
        {
          float dx = (*stored_defects_)[i].center.x - img_x;
          float dy = (*stored_defects_)[i].center.y - img_y;
          float dist = std::sqrt (dx * dx + dy * dy);

          if (dist < nearest_dist)
            {
              nearest_dist = dist;
              nearest_idx = i;
            }
        }

      select_defect (nearest_idx);
    }

  };
} /* namespace waferdefectdetection */