#include "cinebar_generator.h"
#include "utility.h"

namespace cinebar
{
    cv::Mat BuildHorizontalBarcode(const std::vector<cv::Vec3b> &colors, const cinebar_types::InputArgs &args)
    {
        if (colors.empty())
            throw std::runtime_error("cinebar_generator: No colors provided");

        auto bar = app_utility::CreateProgressBar("Generating barcode ", colors.size());
        // setting up parameters for barcode generation
        const int num_colors = static_cast<int>(colors.size());
        int height = static_cast<int>(args.height);
        int bar_width = static_cast<int>(args.bar_w);
        int total_width = static_cast<int>(num_colors) * bar_width;
        cv::Mat barcode(height, total_width, CV_8UC3);

        for (int i = 0; i < num_colors; ++i)
        {
            int x_start = i * bar_width;
            cv::Rect roi(x_start, 0, bar_width, height);
            barcode(roi).setTo(colors[i]);
            app_utility::UpdateProgressBar(bar, i + 1, colors.size());
        }

        return barcode;
    }

    cv::Mat BuildHorizontalBarcodeFromStripes(const std::vector<cv::Mat> &stripes)
    {
        if (stripes.empty())
            throw std::runtime_error("cinebar_generator: No stripes provided");

        // setting up progress spinner
        auto spinner = app_utility::CreateProgressSpinner("Generating barcode");
        std::thread spinner_thread = app_utility::StartSpinnerJob(spinner);
        // concatenate stripes horizontally to form the barcode
        cv::Mat barcode;
        cv::hconcat(stripes, barcode);
        // complete spinner
        app_utility::StopSpinnerJob(spinner, spinner_thread);
        return barcode;
    }
}