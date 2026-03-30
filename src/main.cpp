#include "parser.h"
#include "logger.h"
#include "video_processor.h"
#include "cinebar_generator.h"

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
            // Sample all frames if no interval or nframes is specified
            args.nframes = video_info.frame_count;
            args.interval = 1 / video_info.fps;
        }
        std::cout << video_info.frame_count << std::endl;
        // Ensure nframes does not exceed total frame count
        args.nframes = std::min(args.nframes, video_info.frame_count);

        // Use original height if not specified for horizontal barcodes
        if (args.shape == app_parser::BarcodeShape::Horizontal && args.height <= 0)
        {
            args.height = video_info.height;
            args.width = args.bar_w * args.nframes;
        }
        else
        {
            args.height = args.width = args.bar_w * args.nframes;
        }

        spdlog::info(
            "Cinebar settings:\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}x{}",
            "Input video", args.input_video_path,
            "Output image", args.output_img_path,
            "Sampling interval (s)", args.interval,
            "Frames sampled", args.nframes,
            "Method", app_parser::ToString(args.method),
            "Shape", app_parser::ToString(args.shape),
            "Stripe width (px)", args.bar_w,
            "Output resolution (px)", args.width, args.height);
        spdlog::info(
            "Video info:\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}\n"
            "   {:<22}: {}x{}",
            "Fps", video_info.fps,
            "Duration (s)", video_info.duration,
            "Frames", video_info.frame_count,
            "Size (bytes)", video_info.size,
            "Resolution", video_info.width, video_info.height);

        // Detect letterbox / pillarbox trimming, if specified
        if (args.trim)
        {
            spinner_stop.store(false);
            std::thread spinner_thread(spinner, " Trimming enabled: Detecting letterboxing/pillarboxing...");
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
                "Trimming info:\n"
                "   {:<22}: {}\n"
                "{}",
                "Box type", app_video_processor::ToString(video_info.box_type),
                video_info.box_type != app_video_processor::BoxType::None
                    ? fmt::format(
                          "   {:<22}: {}x{}\n"
                          "   {:<22}: Top={}, Bottom={}, Left={}, Right={}",
                          "Content resolution", content_w, content_h,
                          "Bars (px)", top_bar, bottom_bar, left_bar, right_bar)
                    : fmt::format("   {:<22}: {}x{}", "Content resolution", content_w, content_h));
        }

        if (args.end_frame == -1)
            args.end_frame = video_info.frame_count - 1; // Ensure end_frame is set to the last frame if it was -1

        int n_workers_avail = std::thread::hardware_concurrency();
        if (args.workers > n_workers_avail)
            args.workers = n_workers_avail;

        spinner_stop.store(false);
        std::thread spinner_thread(
            spinner,
            std::string(" Generating cinebar using ") +
                std::to_string(args.workers) +
                " threads: Extracting frames...");
        cv::Mat barcode;

        if (args.method == app_parser::Method::Stripe)
        {
            auto stripes = app_video_processor::ExtractStripes(args, video_info);
            barcode = cinebar::BuildHorizontalBarcodeFromStripes(stripes); // stripes only have horizontal barcode shape
        }
        else
        {
            auto extractor = app_frame_extractor::getColorFunction(args.method);
            auto colors = app_video_processor::ExtractColors(args, video_info, extractor);

            if (args.shape == app_parser::BarcodeShape::Horizontal)
            {
                barcode = cinebar::BuildHorizontalBarcode(colors, args);
            }
            else
            {
                // TODO: Implement circular barcode building
                throw std::runtime_error("circular barcode shape is not implemented yet");
            }
        }

        cv::imwrite(args.output_img_path, barcode);
        spinner_stop.store(true);
        spinner_thread.join();
        spdlog::info("Cinebar generated successfully: {}", args.output_img_path);
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