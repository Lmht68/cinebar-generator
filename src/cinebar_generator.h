#ifndef CINEBAR_GENERATOR_H_
#define CINEBAR_GENERATOR_H_

#include "types.h"

#include <opencv2/opencv.hpp>

#include <atomic>

namespace cinebar
{
    cv::Mat BuildHorizontalBarcode(const std::vector<cv::Vec3b> &colors,
                                   const cinebar_types::InputArgs &args,
                                   std::atomic<int> &progress_current);
                                   std::atomic<int> &progress_current);
    cv::Mat BuildHorizontalBarcodeFromStripes(const std::vector<cv::Mat> &stripes);
}

#endif