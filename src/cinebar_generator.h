#ifndef CINEBAR_GENERATOR_H_
#define CINEBAR_GENERATOR_H_

#include "types.h"
#include "utility.h"

#include <opencv2/opencv.hpp>

namespace cinebar
{
    cv::Mat BuildHorizontalBarcode(const std::vector<cv::Vec3b> &colors,
                                   const cinebar_types::InputArgs &args,
                                   ProgressUpdateCbk on_progress = nullptr);
    cv::Mat BuildHorizontalBarcodeFromStripes(const std::vector<cv::Mat> &stripes,
                                              ProgressCbk on_start = nullptr,
                                              ProgressCbk on_finish = nullptr);
}

#endif