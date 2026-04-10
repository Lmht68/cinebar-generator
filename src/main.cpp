#include "parser.h"
#include "logger.h"
#include "video_processor.h"
#include "cinebar_generator.h"
#include "utility.h"

#include <indicators/cursor_control.hpp>
#include <indicators/progress_spinner.hpp>
#include <indicators/block_progress_bar.hpp>

#include <iostream>
#include <thread>
#include <string>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace
{
    using namespace indicators;

    BlockProgressBar CreateProgressBar()
    {
        return BlockProgressBar{
            option::BarWidth{80},
            option::ShowPercentage{true},
            option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}};
    }

    void AssignProgressBar(BlockProgressBar &bar, const std::string &prefix, size_t max_progress)
    {
        show_console_cursor(false);
        bar.set_option(option::Completed{false});
        bar.set_option(option::PrefixText{prefix});
        bar.set_option(option::ForegroundColor{Color::yellow});
        bar.set_option(option::MaxProgress{max_progress});
    }

    void UpdateProgressBar(BlockProgressBar &bar, size_t current, size_t total)
    {
        bar.set_progress(current);
        bar.set_option(option::PostfixText{
            std::to_string(current) + "/" + std::to_string(total)});

        if (current >= total)
        {
            bar.set_option(option::ForegroundColor{Color::green});
            bar.mark_as_completed();
            show_console_cursor(true);
        }
    }

    void CancelProgressBar(BlockProgressBar &bar)
    {
        bar.set_option(option::ForegroundColor{Color::red});
        bar.mark_as_completed();
        show_console_cursor(true);
    }

    ProgressSpinner CreateProgressSpinner()
    {
        show_console_cursor(false);
        return ProgressSpinner{
            option::ShowPercentage{false},
            option::SpinnerStates{std::vector<std::string>{"▖", "▘", "▝", "▗"}},
            option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}};
    }

    void AssignProgressSpinner(ProgressSpinner &spinner, const std::string &postfix)
    {
        show_console_cursor(false);
        spinner.set_option(option::Completed{false});
        spinner.set_option(option::ShowSpinner{true});
        spinner.set_option(option::PostfixText{postfix});
        spinner.set_option(option::ForegroundColor{Color::yellow});
    }

    std::thread StartSpinnerJob(ProgressSpinner &spinner, std::atomic<bool> &running)
    {
        show_console_cursor(false);

        return std::thread([&spinner, &running]()
                           {
            while (running.load())
            {
                spinner.tick();
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
            } });
    }

    void StopSpinnerJob(std::atomic<bool> &running, ProgressSpinner &spinner, std::thread &t)
    {
        running.store(false);

        spinner.set_option(option::ForegroundColor{Color::green});
        spinner.set_option(option::ShowSpinner{false});
        spinner.mark_as_completed();

        if (t.joinable())
            t.join();

        show_console_cursor(true);
    }
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    // Set Windows console to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

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
        app_parser::ProcessingArgs(args, video_info);

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
            "Method", cinebar_types::ToString(args.method),
            "Shape", cinebar_types::ToString(args.shape),
            "Stripe width (px)", args.bar_w,
            "Output resolution (px)", args.width, args.height);

        // Init progress UI
        std::thread spinner_thread;
        auto spinner = CreateProgressSpinner();
        auto bar = CreateProgressBar();
        std::atomic<bool> spinner_running{false};

        auto start_spinner = [&]()
        {
            spinner_running = true;
            spinner_thread = StartSpinnerJob(spinner, spinner_running);
        };
        auto stop_spinner = [&]()
        {
            if (spinner_thread.joinable())
                StopSpinnerJob(spinner_running, spinner, spinner_thread);
        };
        auto update_progress = [&](size_t current, size_t total)
        {
            UpdateProgressBar(bar, current, total);
        };
        auto cancel_progress = [&]()
        {
            CancelProgressBar(bar);
        };

        // Detect letterbox / pillarbox trimming, if specified
        if (args.trim)
        {
            // create progress spinner for box trimming
            AssignProgressSpinner(spinner, "Trimming enabled: Detecting letterboxing/pillarboxing");
            // run trimming detection
            app_video_processor::DetectVideoBoxType(video_info, start_spinner, stop_spinner);
            int top_bar = 0, bottom_bar = 0, left_bar = 0, right_bar = 0;

            if (video_info.bounds)
            {
                const auto &b = *video_info.bounds;
                top_bar = b.top;
                bottom_bar = static_cast<int>(video_info.height) - b.bottom;
                left_bar = b.left;
                right_bar = static_cast<int>(video_info.width) - b.right;
            }

            int content_w = static_cast<int>(video_info.width) - left_bar - right_bar;
            int content_h = static_cast<int>(video_info.height) - top_bar - bottom_bar;
            spdlog::info(
                "Trimming info:\n"
                "   {:<22}: {}\n"
                "{}",
                "Box type", cinebar_types::ToString(video_info.box_type),
                video_info.box_type != cinebar_types::BoxType::None
                    ? fmt::format(
                          "   {:<22}: {}x{}\n"
                          "   {:<22}: Top={}, Bottom={}, Left={}, Right={}",
                          "Content resolution", content_w, content_h,
                          "Bars (px)", top_bar, bottom_bar, left_bar, right_bar)
                    : fmt::format("   {:<22}: {}x{}", "Content resolution", content_w, content_h));
        }

        cv::Mat barcode;

        if (args.method == cinebar_types::Method::Stripe)
        {
            // create progress bar for processing frames
            AssignProgressBar(bar, "Processing frames ", args.nframes);
            // extract stripes with progress bar callbacks
            std::vector<cv::Mat> stripes;
            stripes = app_video_processor::ExtractStripes(args, video_info, update_progress, cancel_progress);
            // create progress spinner for barcode generation
            AssignProgressSpinner(spinner, "Generating barcode");
            // build barcode from stripes with spinner callbacks
            barcode = cinebar::BuildHorizontalBarcodeFromStripes(stripes, start_spinner, stop_spinner);
        }
        else
        {
            // create progress bar for processing frames
            AssignProgressBar(bar, "Processing frames ", args.nframes);
            // extract colors with progress bar callbacks
            auto extractor = app_frame_extractor::getColorFunction(args.method);
            std::vector<cv::Vec3b> colors;
            colors = app_video_processor::ExtractColors(args, video_info, extractor, update_progress, cancel_progress);
            // create progress bar for barcode generation
            AssignProgressBar(bar, "Generating barcode ", colors.size());

            if (args.shape == cinebar_types::BarcodeShape::Horizontal)
            {
                barcode = cinebar::BuildHorizontalBarcode(colors, args, update_progress);
            }
            else
            {
                // TODO: Implement circular barcode building
                throw std::runtime_error("circular barcode shape is not implemented yet");
            }
        }

        cv::imwrite(args.output_img_path, barcode);
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