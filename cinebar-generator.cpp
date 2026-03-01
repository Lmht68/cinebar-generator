// cinebar-generator.cpp : Defines the entry point for the application.
//

#include "parser.h"
#include "logger.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    try
    {
        app_logger::InitLogger();
        auto args = app_parser::ParseArgs(argc, argv);

        if (args.show_info) {
            spdlog::info("CineBar CLI v0.1.0");
            spdlog::info("OpenCV  : {}.{}.{}",
                CV_VERSION_MAJOR,
                CV_VERSION_MINOR,
                CV_VERSION_REVISION);
            spdlog::info("spdlog  : {}.{}.{}",
                SPDLOG_VER_MAJOR,
                SPDLOG_VER_MINOR,
                SPDLOG_VER_PATCH);
            spdlog::info("CLI11   : {}.{}.{}",
                CLI11_VERSION_MAJOR,
                CLI11_VERSION_MINOR,
                CLI11_VERSION_PATCH);
            return 0;
        }

        cv::Mat image = cv::imread(args.input_video_path);

        if (image.empty()) {
            throw std::runtime_error("Failed to load image.");
        }

        cv::imshow("CineBar - Input Image", image);
        cv::waitKey(0);  // Wait until key press
        cv::destroyAllWindows();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
