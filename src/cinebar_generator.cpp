#include "cinebar_generator.h"

namespace cinebar
{
    cv::Mat BuildHorizontalBarcode(
        const std::vector<cv::Vec3b> &colors,
        int height,
        int bar_width,
        std::atomic<int> &progress_current)
    {
        if (colors.empty())
            throw std::runtime_error("cinebar_generator: No colors provided");

        const int num_colors = static_cast<int>(colors.size());
        int total_width = num_colors * bar_width;

        cv::Mat barcode(height, total_width, CV_8UC3);

        for (int i = 0; i < num_colors; ++i)
        {
            int x_start = i * bar_width;
            cv::Rect roi(x_start, 0, bar_width, height);
            barcode(roi).setTo(colors[i]);
            progress_current = i + 1;
        }

        return barcode;
    }

    cv::Mat BuildHorizontalBarcodeFromStripes(
        const std::vector<cv::Mat> &stripes)
    {
        if (stripes.empty())
            throw std::runtime_error("cinebar_generator: No stripes provided");
        cv::Mat barcode;
        cv::hconcat(stripes, barcode);
        return barcode;
    }
}