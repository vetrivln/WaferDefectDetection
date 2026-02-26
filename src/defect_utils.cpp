#include "defect_utils.h"

std::string
to_std_string (System::String^ s)
{
  return msclr::interop::marshal_as<std::string> (s);
}

System::Drawing::Bitmap^
mat_to_bitmap (const cv::Mat& mat)
{
  cv::Mat bgr;
  if (mat.channels () == 1)
    cv::cvtColor (mat, bgr, cv::COLOR_GRAY2BGR);
  else
    bgr = mat;

  cv::Mat rgb;
  cv::cvtColor (bgr, rgb, cv::COLOR_BGR2RGB);

  System::Drawing::Bitmap^ bmp
    = gcnew System::Drawing::Bitmap 
    (
        rgb.cols, rgb.rows,
        System::Drawing::Imaging::PixelFormat::Format24bppRgb
    );

  System::Drawing::Rectangle rect (0, 0, bmp->Width, bmp->Height);
  System::Drawing::Imaging::BitmapData^ bmp_data
    = bmp->LockBits (rect,
                     System::Drawing::Imaging::ImageLockMode::WriteOnly,
                     bmp->PixelFormat);

  for (int y = 0; y < rgb.rows; y++)
    memcpy (
        (char*)bmp_data->Scan0.ToPointer () + y * bmp_data->Stride,
        rgb.ptr (y),
        rgb.cols * 3);

  bmp->UnlockBits (bmp_data);
  return bmp;
}