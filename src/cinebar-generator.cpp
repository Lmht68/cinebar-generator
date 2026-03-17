// cinebar-generator.cpp : Defines the entry point for the application.
//

#include "parser.h"
#include "logger.h"
#include "video_processor.h"

#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>

namespace
{
    // Global atomic flag to signal spinner to stop
    std::atomic<bool> spinner_stop(false);

    // Spinner function
    void spinner(const std::string &prefix)
    {
        const std::string spin_chars = "|/-\\";
        int i = 0;
        while (!spinner_stop.load())
        {
            std::cout << "\r" << prefix << spin_chars[i % spin_chars.size()] << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            ++i;
        }
        // Clear line when finished
        std::cout << "\r" << std::string(prefix.size() + 2, ' ') << "\r";
    }
}

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
        else if (args.nframes > 0)
        {
            // Compute sampling FPS from desired number of frames
            double duration = static_cast<double>(video_info.frame_count) / video_info.fps;
            args.interval = args.nframes / duration;
        }
        else
        {
            args.nframes = video_info.frame_count; // Sample all frames if no interval or nframes is specified
            args.interval = 1 / video_info.fps;
        }

        args.nframes = std::min(args.nframes, video_info.frame_count); // Ensure nframes does not exceed total frame count

        if (args.width <= 0)
            args.width = 1; // Default width is 1 pixel per frame if not specified

        if (args.height <= 0)
            args.height = video_info.height; // Use original height if not specified

        spdlog::info(
            "Video processing settings:\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}",
            "Input video", args.input_video_path,
            "Output image", args.output_img_path,
            "Fps", video_info.fps,
            "Sampling interval (s)", args.interval,
            "Frames sampled", args.nframes,
            "Shape", app_parser::ToString(args.shape));

        // Detect letterbox / pillarbox trimming, if specified
        spinner_stop.store(false);
        std::thread spinner_thread(spinner, " Video box info:... ");
        app_video_processor::DetectVideoBoxType(video_info);
        int top_bar = 0, bottom_bar = 0, left_bar = 0, right_bar = 0;

        if (video_info.bounds)
        {
            const auto &b = *video_info.bounds;
            top_bar = b.top;
            bottom_bar = video_info.height - b.bottom;
            left_bar = b.left;
            right_bar = video_info.width - b.right;
        }

        int content_w = video_info.width - left_bar - right_bar;
        int content_h = video_info.height - top_bar - bottom_bar;
        spinner_stop.store(true);
        spinner_thread.join();
        spdlog::info(
            "Video box info:\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}x{}\n"
            "{}",
            "Box type", app_video_processor::ToString(video_info.box_type),
            "Original resolution", video_info.width, video_info.height,
            video_info.box_type != app_video_processor::BoxType::None
                ? fmt::format(
                      "   {:<22}: {}x{}\n"
                      "   {:<22}: Top={}, Bottom={}, Left={}, Right={}",
                      "Content resolution", content_w, content_h,
                      "Bars (pixels)", top_bar, bottom_bar, left_bar, right_bar)
                : fmt::format("   {:<22}: {}x{}", "Content resolution", content_w, content_h));
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