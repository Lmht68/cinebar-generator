#ifndef CINEBAR_GENERATOR_H_
#define CINEBAR_GENERATOR_H_

#include "types.h"

#include <opencv2/opencv.hpp>

#include <atomic>

namespace cinebar
{
    inline constexpr int kCircularScaleFactor = 10;

    cv::Mat BuildHorizontalBarcode(const std::vector<cv::Vec3b> &colors,
                                   int height,
                                   int bar_width,
                                   std::atomic<int> &progress_current);
    cv::Mat BuildCircularBarcode(const std::vector<cv::Vec3b> &colors,
                                 int img_size,
                                 std::atomic<int> &progress_current);
    cv::Mat BuildHorizontalBarcodeFromStripes(const std::vector<cv::Mat> &stripes);
}

#endif