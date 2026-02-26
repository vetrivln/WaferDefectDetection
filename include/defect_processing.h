#pragma once

#include "defect_utils.h"
#include <vector>

cv::Mat
extract_lens_mask (const cv::Mat& gray);

cv::Mat
correct_illumination (const cv::Mat& gray, const cv::Mat& mask, int blur_size);

cv::Mat
detect_defects (const cv::Mat& corrected, const cv::Mat& mask, int threshold);

std::vector<Defect>
analyze_defects (const cv::Mat& defect_mask);

cv::Mat
build_annotated_display (const cv::Mat& corrected, const cv::Mat& mask,
                         const std::vector<Defect>& defects, bool pass, 
                         float ratio);