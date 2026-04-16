#include "cinebar_generator.h"

#include <vector>
#include <cmath>
#include <algorithm>

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

    cv::Mat BuildCircularBarcode(
        const std::vector<cv::Vec3b> &colors,
        int img_size,
        std::atomic<int> &progress_current)
    {
        if (colors.empty() || img_size <= 0)
            return cv::Mat();
        const int cx = img_size / 2;
        const int cy = img_size / 2;
        const float max_radius = static_cast<float>(cx);
        const size_t total = colors.size();
        const float inv_radius_step = static_cast<float>(total) / max_radius;

        cv::Mat result(img_size, img_size, CV_8UC4);

        for (int y = 0; y < img_size; ++y)
        {
            int dy = y - cy;

            for (int x = 0; x < img_size; ++x)
            {
                int dx = x - cx;
                float r = std::sqrt(static_cast<float>(dx * dx + dy * dy));

                cv::Vec4b &px = result.at<cv::Vec4b>(y, x);

                if (r > max_radius)
                {
                    // Transparent outside circle
                    px = cv::Vec4b(0, 0, 0, 0);
                }
                else
                {
                    // Map radius to color index
                    int idx = static_cast<int>(r * inv_radius_step);
                    if (idx >= static_cast<int>(total))
                        idx = static_cast<int>(total) - 1;
                    const cv::Vec3b &c = colors[idx];
                    px[0] = c[0]; // B
                    px[1] = c[1]; // G
                    px[2] = c[2]; // R
                    px[3] = 255;  // A
                }
            }

            progress_current = y + 1;
        }

        return result;
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