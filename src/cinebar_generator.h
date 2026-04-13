#ifndef CINEBAR_GENERATOR_H_
#define CINEBAR_GENERATOR_H_

#include "types.h"

#include <opencv2/opencv.hpp>

namespace cinebar
{
    cv::Mat BuildHorizontalBarcode(const std::vector<cv::Vec3b> &colors,
                                   const cinebar_types::InputArgs &args,
                                   std::atomic<size_t> &progress_current);
    cv::Mat BuildHorizontalBarcodeFromStripes(const std::vector<cv::Mat> &stripes);
}

#endif