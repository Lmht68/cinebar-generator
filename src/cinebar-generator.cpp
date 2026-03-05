// cinebar-generator.cpp : Defines the entry point for the application.
//

#include "parser.h"
#include "logger.h"
#include "video_processor.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    try
    {
        app_logger::InitLogger();
        auto args = app_parser::ParseArgs(argc, argv);

        if (args.show_info)
        {
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

        auto video_info = app_video_processor::LoadVideoInfo(args.input_video_path);

        if (args.interval > 0.0)
        {
            args.nframes = app_video_processor::NframesFromInterval(video_info, args.interval);
        }
        else if (args.nframes <= 0)
        {
            args.nframes = video_info.frame_count; // Sample all frames if no interval or nframes is specified
        }

        args.nframes = std::min(args.nframes, video_info.frame_count); // Ensure nframes does not exceed total frame count

        if (args.width <= 0)
            args.width = 1; // Default width is 1 pixel per frame if not specified

        if (args.height <= 0)
            args.height = video_info.height; // Use original height if not specified

        spdlog::info("input video: {}", args.input_video_path);
        spdlog::info("output image: {}", args.output_img_path);
        spdlog::info("interval: {} seconds", args.interval);
        spdlog::info("frames to sample: {}", args.nframes);
        spdlog::info("width: {}", args.width);
        spdlog::info("height: {}", args.height);
    }
    catch (const CLI::ParseError &pe)
    {
        spdlog::error("parser: {}", pe.what());
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
