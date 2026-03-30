#include "parser.h"

#include <opencv2/opencv.hpp>

namespace cinebar
{
    cv::Mat BuildHorizontalBarcode(const std::vector<cv::Vec3b> &colors, const app_parser::InputArgs &args);
    cv::Mat BuildHorizontalBarcodeFromStripes(const std::vector<cv::Mat> &stripes);
}