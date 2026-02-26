#pragma once

#include <opencv2/opencv.hpp>
#include <msclr/marshal_cppstd.h>

struct Defect
{
	cv::Point2f center;
	cv::Rect boundingBox;
	float area;
	float ar;
	std::string type;
};

std::string
to_std_string (System::String^ s);

System::Drawing::Bitmap^
mat_to_bitmap (const cv::Mat& mat);