#include "cinebar_generator.h"

namespace cinebar
{
    cv::Mat BuildHorizontalBarcode(
        const std::vector<cv::Vec3b> &colors,
        const cinebar_types::InputArgs &args,
        ProgressUpdateCbk on_progress)
    {
        if (colors.empty())
            throw std::runtime_error("cinebar_generator: No colors provided");

        const int num_colors = static_cast<int>(colors.size());
        int height = static_cast<int>(args.height);
        int bar_width = static_cast<int>(args.bar_w);
        int total_width = num_colors * bar_width;

        cv::Mat barcode(height, total_width, CV_8UC3);

        for (int i = 0; i < num_colors; ++i)
        {
            int x_start = i * bar_width;
            cv::Rect roi(x_start, 0, bar_width, height);
            barcode(roi).setTo(colors[i]);

            if (on_progress)
                on_progress(i + 1, colors.size());
        }

        return barcode;
    }

    cv::Mat BuildHorizontalBarcodeFromStripes(
        const std::vector<cv::Mat> &stripes,
        ProgressCbk on_start,
        ProgressCbk on_finish)
    {
        if (stripes.empty())
            throw std::runtime_error("cinebar_generator: No stripes provided");

        if (on_start)
            on_start();

        cv::Mat barcode;
        cv::hconcat(stripes, barcode);

        if (on_finish)
            on_finish();

        return barcode;
    }
}