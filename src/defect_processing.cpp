#include "defect_processing.h"

cv::Mat
extract_lens_mask (const cv::Mat& gray)
{
  cv::Mat mask;
  cv::threshold (gray, mask, 8, 255, cv::THRESH_BINARY);

  auto kernel = cv::getStructuringElement (cv::MORPH_ELLIPSE, { 15, 15 });
  cv::morphologyEx (mask, mask, cv::MORPH_CLOSE, kernel);
  cv::morphologyEx (mask, mask, cv::MORPH_OPEN, kernel);

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours (mask, contours,
                    cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  cv::Mat clean_mask = cv::Mat::zeros (gray.size (), CV_8U);
  if (!contours.empty ())
    {
      int largest = 0;
      double max_area = 0.0;

      for (int i = 0; i < (int)contours.size (); i++)
        {
          double a = cv::contourArea (contours[i]);
          if (a > max_area)
            {
              max_area = a;
              largest = i;
            }
        }

      cv::drawContours (clean_mask, contours, largest, 255, cv::FILLED);
    }

  return clean_mask;
}

cv::Mat
correct_illumination (const cv::Mat& gray,
                      const cv::Mat& mask,
                      int blur_size)
{
  if (blur_size % 2 == 0)
    blur_size++;

  cv::Mat float_gray;
  gray.convertTo (float_gray, CV_32F);

  cv::Mat background;
  cv::GaussianBlur (float_gray, background, { blur_size, blur_size }, 0);

  cv::Mat corrected;
  cv::divide (float_gray + 1.0f, background + 1.0f, corrected);
  cv::normalize (corrected, corrected, 0, 255, cv::NORM_MINMAX, CV_8U, mask);

  return corrected;
}

cv::Mat
detect_defects (const cv::Mat& corrected,
                const cv::Mat& mask,
                int threshold)
{
  cv::Mat enhanced;
  auto clahe = cv::createCLAHE (3.0, { 8, 8 });
  clahe->apply (corrected, enhanced);

  cv::Mat tophat;
  auto kernel = cv::getStructuringElement (cv::MORPH_ELLIPSE, { 7, 7 });
  cv::morphologyEx (enhanced, tophat, cv::MORPH_TOPHAT, kernel);

  cv::Mat defect_mask;
  cv::threshold (tophat, defect_mask, threshold, 255, cv::THRESH_BINARY);

  auto noise_kernel = cv::getStructuringElement (cv::MORPH_ELLIPSE, { 3, 3 });
  cv::morphologyEx (defect_mask, defect_mask, cv::MORPH_OPEN, noise_kernel);

  cv::bitwise_and (defect_mask, mask, defect_mask);

  return defect_mask;
}

std::vector<Defect>
analyze_defects (const cv::Mat& defect_mask)
{
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours (defect_mask, contours,
                    cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

  std::vector<Defect> defects;

  for (auto& c : contours)
    {
      float area = (float)cv::contourArea (c);
      if (area < 2.0f)
        continue;

      Defect d;
      d.area = area;
      d.boundingBox = cv::boundingRect (c);

      auto moments = cv::moments (c);
      d.center = { (float)(moments.m10 / moments.m00),
                   (float)(moments.m01 / moments.m00) };

      float w = (float)d.boundingBox.width;
      float h = (float)d.boundingBox.height;
      float ar = w / std::max<float> (h, 1.0f);
      d.ar = ar;

      bool is_elongated = (ar > 2.5f || ar <= 0.70f);
      bool is_large_enough = (area > 5.0f);

      if (is_elongated && is_large_enough)
        d.type = "scratch";
      else if (area > 150.0f)
        d.type = "cluster";
      else
        d.type = "speck";

      defects.push_back (d);
    }

  return defects;
}

cv::Mat
build_annotated_display (const cv::Mat& corrected,
                         const cv::Mat& mask,
                         const std::vector<Defect>& defects,
                         bool pass,
                         float ratio)
{
  cv::Mat display;
  cv::cvtColor (corrected, display, cv::COLOR_GRAY2BGR);

  std::vector<std::vector<cv::Point>> contours;
  cv::findContours (mask, contours,
                    cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
  cv::drawContours (display, contours, -1, { 0, 255, 0 }, 3);

  for (int i = 0; i < (int)defects.size (); i++)
    {
      const auto& d = defects[i];

      cv::Scalar color
        = (d.type == "scratch") ? cv::Scalar (0, 0, 255)
        : (d.type == "cluster") ? cv::Scalar (0, 165, 255)
        : cv::Scalar (255, 0, 255);

      int radius = std::max<float> (8, (int)std::sqrt (d.area) + 4);
      cv::circle (display, d.center, radius, color, 2);
      cv::putText (display,
                   std::to_string (i + 1),
                   { (int)d.center.x + radius + 2, (int)d.center.y + 4 },
                   cv::FONT_HERSHEY_SIMPLEX, 0.4, color, 1);
    }

  return display;
}