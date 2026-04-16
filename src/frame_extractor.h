#ifndef FRAME_EXTRACTOR_H_
#define FRAME_EXTRACTOR_H_

#include "parser.h"

#include <opencv2/opencv.hpp>

namespace app_frame_extractor
{
    inline constexpr int kNumkMean = 3;                 // Number of clusters for k-means
    inline constexpr int kMaxSamples = 4096;            // Maximum samples for k-means
    inline constexpr int kResizeDim = 128;              // Dimension to resize frames for processing
    inline constexpr int kTermCriteriaMaxIter = 10;     // Max iterations for k-means
    inline constexpr double kTermCriteriaEpsilon = 1.0; // Epsilon for k
    inline constexpr int kKMeansMaxAttempts = 3;

    inline constexpr int kBinsPerChannel = 8; // power of 2 is required for bit-shift optimization
    inline constexpr int kBitsPerChannel = 3; // log2(kBinsPerChannel)
    inline constexpr int kShift = 8 - kBitsPerChannel;
    inline constexpr int kHistSize = kBinsPerChannel * kBinsPerChannel * kBinsPerChannel;

    inline constexpr int kHueBins = 180;
    inline constexpr float kHueRange[2] = {0.f, 180.f};
    inline constexpr int kHueChannel = 0;
    inline constexpr int kMinHue = 0;
    inline constexpr int kMaxHue = 180;
    inline constexpr int kMinSaturation = 50;
    inline constexpr int kMaxSaturation = 255;
    inline constexpr int kMinValue = 50;
    inline constexpr int kMaxValue = 255;

    template <int interpolation>
    cv::Mat ExtractFrameStripe(const cv::Mat &frame, size_t width)
    {
        cv::Mat stripe;
        cv::resize(frame, stripe, cv::Size(static_cast<int>(width), frame.rows), 0, 0, interpolation);
        return stripe;
    }

    inline cv::Mat ExtractMeanFrameStripe(const cv::Mat &frame, size_t width)
    {
        return ExtractFrameStripe<cv::INTER_AREA>(frame, width);
    }

    inline cv::Mat ExtractSmoothedFrameStripe(const cv::Mat &frame, size_t width)
    {
        return ExtractFrameStripe<cv::INTER_LINEAR>(frame, width);
    }

    cv::Vec3b ExtractSmoothedColor(const cv::Mat &frame);
    cv::Vec3b ExtractColorMean(const cv::Mat &frame);
    cv::Vec3b ExtractColorkMeans(const cv::Mat &frame);
    cv::Vec3b ExtractColorHistogram(const cv::Mat &frame);
    cv::Vec3b ExtractDominantHue(const cv::Mat &frame);
}

#endif