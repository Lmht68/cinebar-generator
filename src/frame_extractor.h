#ifndef FRAME_EXTRACTOR_H_
#define FRAME_EXTRACTOR_H_

#include "parser.h"

#include <opencv2/opencv.hpp>

namespace app_frame_extractor
{
    constexpr int kNumkMean = 3;                 // Number of clusters for k-means
    constexpr int kMaxSamples = 4096;            // Maximum samples for k-means
    constexpr int kResizeDim = 128;              // Dimension to resize frames for processing
    constexpr int kTermCriteriaMaxIter = 10;     // Max iterations for k-means
    constexpr double kTermCriteriaEpsilon = 1.0; // Epsilon for k
    constexpr int kKMeansMaxAttempts = 3;

    constexpr int kBinsPerChannel = 8; // power of 2 is required for bit-shift optimization
    constexpr int kBitsPerChannel = 3; // log2(kBinsPerChannel)
    constexpr int kShift = 8 - kBitsPerChannel;
    constexpr int kHistSize = kBinsPerChannel * kBinsPerChannel * kBinsPerChannel;

    constexpr int kHueBins = 180;
    constexpr float kHueRange[2] = {0.f, 180.f};
    constexpr int kHueChannel = 0;
    constexpr int kMinHue = 0;
    constexpr int kMaxHue = 180;
    constexpr int kMinSaturation = 50;
    constexpr int kMaxSaturation = 255;
    constexpr int kMinValue = 50;
    constexpr int kMaxValue = 255;

    cv::Mat ExtractFrameStripe(const cv::Mat &frame, size_t width);
    cv::Vec3b ExtractSmoothedColor(const cv::Mat &frame);
    cv::Vec3b ExtractColorMean(const cv::Mat &frame);
    cv::Vec3b ExtractColorkMeans(const cv::Mat &frame);
    cv::Vec3b ExtractColorHistogram(const cv::Mat &frame);
    cv::Vec3b ExtractDominantHue(const cv::Mat &frame);
}

#endif