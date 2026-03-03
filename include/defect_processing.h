#pragma once

#include "defect_utils.h"
#include <vector>
#include <opencv2/dnn.hpp>

cv::Mat extract_lens_mask (const cv::Mat& gray);
cv::Mat correct_illumination (const cv::Mat& gray, const cv::Mat& mask, int blur_size);
cv::Mat detect_defects (const cv::Mat& corrected, const cv::Mat& mask, int threshold);

class DefectClassifier
{
public:
  DefectClassifier(const std::string& model_path);
  std::string classify(const cv::Mat& defect_roi);
  
private:
  cv::dnn::Net net_;
  std::vector<std::string> classes_ = {"scratch", "particle", "crack", "edge", "other"};
};

cv::Mat build_annotated_display (const cv::Mat& corrected, const cv::Mat& mask,
                                 const std::vector<Defect>& defects, bool pass, float ratio);