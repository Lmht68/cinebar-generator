#include "cinebar_generator.h"
#include "parser.h"

namespace cinebar
{
    cv::Mat BuildHorizontalBarcode(const std::vector<cv::Vec3b> &colors, const cinebar_types::InputArgs &args)
    {
        if (colors.empty())
            throw std::runtime_error("cinebar_generator: No colors provided");

        int total_width = static_cast<int>(colors.size()) * args.bar_w;
        cv::Mat barcode(args.height, total_width, CV_8UC3);

        for (int y = 0; y < args.height; ++y)
        {
            cv::Vec3b *row = barcode.ptr<cv::Vec3b>(y);

            for (int i = 0; i < colors.size(); ++i)
            {
                int x_start = i * args.bar_w;
                for (int dx = 0; dx < args.bar_w; ++dx)
                    row[x_start + dx] = colors[i];
            }
        }

        return barcode;
    }

    cv::Mat BuildHorizontalBarcodeFromStripes(const std::vector<cv::Mat> &stripes)
    {
        if (stripes.empty())
            throw std::runtime_error("cinebar_generator: No stripes provided");

        cv::Mat barcode;
        cv::hconcat(stripes, barcode);
        return barcode;
    }
}