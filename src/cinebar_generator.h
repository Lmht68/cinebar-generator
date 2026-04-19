#ifndef CINEBAR_GENERATOR_H_
#define CINEBAR_GENERATOR_H_

#include "types.h"

#include <opencv2/opencv.hpp>

#include <atomic>

/**
 * @file cinebar_generator.h
 * @brief Barcode image constructor for color and stripe outputs.
 */

namespace cinebar
{
    /** @brief Multiplier used when deriving circular barcode image dimensions. */
    inline constexpr int kCircularScaleFactor = 10;

    /**
     * @brief Builds a horizontal color barcode by filling vertical bands.
     *
     * @param colors Sampled colors in barcode order.
     * @param height Output image height in pixels.
     * @param bar_width Width of each vertical band in pixels.
     * @param progress_current Atomic progress counter updated after each band.
     * @return Generated barcode image in BGR format.
     */
    cv::Mat BuildHorizontalBarcode(const std::vector<cv::Vec3b> &colors,
                                   int height,
                                   int bar_width,
                                   std::atomic<int> &progress_current);

    /**
     * @brief Builds a circular barcode where radius maps to sampling order.
     *
     * @param colors Sampled colors in barcode order from center to edge.
     * @param img_size Output image width and height in pixels.
     * @param progress_current Atomic progress counter updated after each row.
     * @return Generated barcode image in BGRA format.
     */
    cv::Mat BuildCircularBarcode(const std::vector<cv::Vec3b> &colors,
                                 int img_size,
                                 std::atomic<int> &progress_current);

    /**
     * @brief Concatenates precomputed frame stripes into one horizontal barcode.
     *
     * @param stripes Stripe images ordered by sample time.
     * @return Generated barcode image in BGR format.
     */
    cv::Mat BuildHorizontalBarcodeFromStripes(const std::vector<cv::Mat> &stripes);
}

#endif
