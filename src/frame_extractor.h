#ifndef FRAME_EXTRACTOR_H_
#define FRAME_EXTRACTOR_H_

#include "parser.h"

#include <opencv2/opencv.hpp>

namespace app_frame_extractor
{
    constexpr int kNumkMean = 3;                 // Number of clusters for k-means
    constexpr int kTermCriteriaMaxIter = 10;     // Max iterations for k-means
    constexpr double kTermCriteriaEpsilon = 1.0; // Epsilon for k
    constexpr int kKMeansMaxAttempts = 3;

    cv::Mat ExtractFrameStripe(const cv::Mat &frame, int width);
    cv::Vec3b ExtractSmoothedColor(const cv::Mat &frame);
    cv::Vec3b ExtractColorMean(const cv::Mat &frame);
    cv::Vec3b ExtractColorkMeans(const cv::Mat &frame);
    cv::Vec3b ExtractDominantHue(const cv::Mat &frame);

    using ColorFunc = cv::Vec3b (*)(const cv::Mat &);
    using StripeFunc = cv::Mat (*)(const cv::Mat &, int);
    const std::map<cinebar_types::Method, ColorFunc> kColorExtractorMap{
        {cinebar_types::Method::Avg, ExtractColorMean},
        {cinebar_types::Method::Smoothed, ExtractSmoothedColor},
        {cinebar_types::Method::KMeans, ExtractColorkMeans},
        {cinebar_types::Method::HSV, ExtractDominantHue}};

    ColorFunc getColorFunction(cinebar_types::Method method);
    StripeFunc getStripeFunction();
}

#endif