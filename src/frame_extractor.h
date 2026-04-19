#ifndef FRAME_EXTRACTOR_H_
#define FRAME_EXTRACTOR_H_

#include "parser.h"

#include <opencv2/opencv.hpp>

/**
 * @file frame_extractor.h
 * @brief Stripe and dominant-color extraction helpers.
 */

namespace app_frame_extractor
{
    /** @brief Number of clusters used by the lightweight k-means estimator. */
    inline constexpr int kNumkMean = 3; // Number of clusters for k-means
    /** @brief Maximum number of sampled pixels evaluated by k-means. */
    inline constexpr int kMaxSamples = 4096; // Maximum samples for k-means
    /** @brief Square resize dimension used before color analysis. */
    inline constexpr int kResizeDim = 128; // Dimension to resize frames for processing
    /** @brief Maximum k-means iteration count. */
    inline constexpr int kTermCriteriaMaxIter = 10; // Max iterations for k-means
    /** @brief Convergence epsilon used by k-means-style processing. */
    inline constexpr double kTermCriteriaEpsilon = 1.0; // Epsilon for k
    /** @brief Maximum restart attempts allowed for k-means. */
    inline constexpr int kKMeansMaxAttempts = 3;

    /** @brief Histogram bucket count per channel for RGB quantization. */
    inline constexpr int kBinsPerChannel = 8; // power of 2 is required for bit-shift optimization
    /** @brief Number of bits needed to encode each quantized channel. */
    inline constexpr int kBitsPerChannel = 3; // log2(kBinsPerChannel)
    /** @brief Bit shift used when quantizing 8-bit channels into histogram bins. */
    inline constexpr int kShift = 8 - kBitsPerChannel;
    /** @brief Total number of bins in the 3-channel color histogram. */
    inline constexpr int kHistSize = kBinsPerChannel * kBinsPerChannel * kBinsPerChannel;

    /** @brief Number of hue buckets used for dominant-hue extraction. */
    inline constexpr int kHueBins = 180;
    /** @brief Hue range passed to OpenCV histogram APIs. */
    inline constexpr float kHueRange[2] = {0.f, 180.f};
    /** @brief HSV channel index used when building the hue histogram. */
    inline constexpr int kHueChannel = 0;
    /** @brief Minimum hue value included in the hue mask. */
    inline constexpr int kMinHue = 0;
    /** @brief Maximum hue value included in the hue mask. */
    inline constexpr int kMaxHue = 180;
    /** @brief Minimum saturation threshold for the hue mask. */
    inline constexpr int kMinSaturation = 50;
    /** @brief Maximum saturation threshold for the hue mask. */
    inline constexpr int kMaxSaturation = 255;
    /** @brief Minimum value threshold for the hue mask. */
    inline constexpr int kMinValue = 50;
    /** @brief Maximum value threshold for the hue mask. */
    inline constexpr int kMaxValue = 255;

    /**
     * @brief Resizes a frame horizontally to extract a representative stripe.
     *
     * @tparam interpolation OpenCV interpolation mode used during resizing.
     * @param frame Source frame in BGR format.
     * @param width Target stripe width in pixels.
     * @return Resized stripe image.
     */
    template <int interpolation>
    cv::Mat ExtractFrameStripe(const cv::Mat &frame, int width)
    {
        cv::Mat stripe;
        cv::resize(frame, stripe, cv::Size(static_cast<int>(width), frame.rows), 0, 0, interpolation);
        return stripe;
    }

    /**
     * @brief Extracts a stripe using area resampling for stronger averaging.
     *
     * @param frame Source frame in BGR format.
     * @param width Target stripe width in pixels.
     * @return Stripe image for concatenation.
     */
    inline cv::Mat ExtractMeanFrameStripe(const cv::Mat &frame, int width)
    {
        return ExtractFrameStripe<cv::INTER_AREA>(frame, width);
    }

    /**
     * @brief Extracts a stripe using linear interpolation for smoother output.
     *
     * @param frame Source frame in BGR format.
     * @param width Target stripe width in pixels.
     * @return Stripe image for concatenation.
     */
    inline cv::Mat ExtractSmoothedFrameStripe(const cv::Mat &frame, int width)
    {
        return ExtractFrameStripe<cv::INTER_LINEAR>(frame, width);
    }

    /**
     * @brief Collapses a frame to a single color by resizing it to one pixel.
     *
     * @param frame Source frame in BGR format.
     * @return Dominant smoothed BGR color.
     */
    cv::Vec3b ExtractSmoothedColor(const cv::Mat &frame);

    /**
     * @brief Computes the arithmetic mean color of a frame.
     *
     * @param frame Source frame in BGR format.
     * @return Mean BGR color.
     */
    cv::Vec3b ExtractColorMean(const cv::Mat &frame);

    /**
     * @brief Estimates a dominant color using a lightweight k-means pass.
     *
     * @param frame Source frame in BGR format.
     * @return Dominant cluster center as a BGR color.
     */
    cv::Vec3b ExtractColorkMeans(const cv::Mat &frame);

    /**
     * @brief Finds the most populated quantized RGB histogram bin.
     *
     * @param frame Source frame in BGR format.
     * @return Center color of the dominant histogram bin in BGR order.
     */
    cv::Vec3b ExtractColorHistogram(const cv::Mat &frame);

    /**
     * @brief Finds the dominant hue in HSV space and converts it back to BGR.
     *
     * @param frame Source frame in BGR format.
     * @return BGR color built from the dominant hue with full saturation and value.
     */
    cv::Vec3b ExtractDominantHue(const cv::Mat &frame);
}

#endif
