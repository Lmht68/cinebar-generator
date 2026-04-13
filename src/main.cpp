#include "parser.h"
#include "logger.h"
#include "video_processor.h"
#include "cinebar_generator.h"

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

    void AssignProgressBar(BlockProgressBar &bar, const std::string &prefix, size_t max_progress, size_t &progress_total)
    {
        show_console_cursor(false);
        bar.set_option(option::Completed{false});
        bar.set_option(option::PrefixText{prefix});
        bar.set_option(option::ForegroundColor{Color::yellow});
        bar.set_option(option::MaxProgress{max_progress});
        progress_total = max_progress;
    }

    void UpdateProgressBar(BlockProgressBar &bar, size_t current, size_t total)
    {
        bar.set_progress(current);
        bar.set_option(option::PostfixText{
            std::to_string(current) + "/" + std::to_string(total)});
    }

    std::thread StartProgressJob(BlockProgressBar &bar,
                                 std::atomic<size_t> &current,
                                 std::atomic<bool> &running,
                                 size_t total)
    {
        return std::thread([&running, &current, &bar, total]()
                           {
            while (running.load())
            {
                UpdateProgressBar(bar, current.load(), total);
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
            }

            // final update
            UpdateProgressBar(bar, total, total);

            if (current.load() == total)
            {
                bar.set_option(option::ForegroundColor{Color::green});
            } else {
                bar.set_option(option::ForegroundColor{Color::red});
            }

            bar.mark_as_completed();
            show_console_cursor(true); });
    }

    void StopProgressJob(std::atomic<bool> &running, std::thread &t)
    {
        running.store(false);
        if (t.joinable())
            t.join();
    }

    ProgressSpinner CreateProgressSpinner()
    {
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
            }

            spinner.set_option(option::ForegroundColor{Color::green});
            spinner.set_option(option::ShowSpinner{false});
            spinner.mark_as_completed();
            show_console_cursor(true); });
    }

    void StopSpinnerJob(std::atomic<bool> &running, ProgressSpinner &spinner, std::thread &t)
    {
        running.store(false);
        if (t.joinable())
            t.join();
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
        std::atomic<bool> spinner_running{false};
        auto spinner = CreateProgressSpinner();

        auto start_spinner = [&]()
        {
            spinner_running = true;
            spinner_thread = StartSpinnerJob(spinner, spinner_running);
        };
        auto stop_spinner = [&]()
        {
            StopSpinnerJob(spinner_running, spinner, spinner_thread);
        };

        std::thread progress_thread;
        std::atomic<size_t> progress_current{0};
        std::atomic<bool> progress_running{false};
        size_t progress_total = 0;
        auto bar = CreateProgressBar();

        auto start_progress = [&]()
        {
            progress_running = true;
            progress_current = 0;
            progress_thread = StartProgressJob(bar, progress_current, progress_running, progress_total);
        };

        auto stop_progress = [&]()
        {
            StopProgressJob(progress_running, progress_thread);
        };
        auto update_progress = [&](size_t current)
        {
            progress_current = current;
        };

        // Detect letterbox / pillarbox trimming, if specified
        if (args.trim)
        {
            // create progress spinner for box trimming
            AssignProgressSpinner(spinner, "Trimming enabled: Detecting letterboxing/pillarboxing");
            // run trimming detection
            start_spinner();
            app_video_processor::DetectVideoBoxType(video_info);
            stop_spinner();
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
            AssignProgressBar(bar, "Processing frames ", args.nframes, progress_total);
            // extract stripes with progress bar callbacks
            start_progress();
            std::vector<cv::Mat> stripes = app_video_processor::ExtractStripes(args, video_info, progress_current);
            stop_progress();
            // create progress spinner for barcode generation
            AssignProgressSpinner(spinner, "Generating barcode");
            // build barcode from stripes with spinner callbacks
            start_spinner();
            barcode = cinebar::BuildHorizontalBarcodeFromStripes(stripes);
            stop_spinner();
        }
        else
        {
            // create progress bar for processing frames
            AssignProgressBar(bar, "Processing frames ", args.nframes, progress_total);
            // extract colors with progress bar callbacks
            start_progress();
            std::vector<cv::Vec3b> colors = app_video_processor::ExtractColorsDispatch(args, video_info, progress_current);
            stop_progress();
            // create progress bar for barcode generation
            AssignProgressBar(bar, "Generating barcode ", colors.size(), progress_total);
            start_progress();

            if (args.shape == cinebar_types::BarcodeShape::Horizontal)
            {
                barcode = cinebar::BuildHorizontalBarcode(colors, args, progress_current);
            }
            else
            {
                // TODO: Implement circular barcode building
                throw std::runtime_error("circular barcode shape is not implemented yet");
            }

            stop_progress();
        }

        cv::imwrite(args.output_img_path, barcode);
        spdlog::info("Cinebar generated successfully: {}", args.output_img_path);
    }
    catch (const CLI::ParseError &pe)
    {
        spdlog::error("parser: {}", pe.what());
        return 1;
    }
    catch (const cv::Exception &ce)
    {
        spdlog::error("OpenCV error: {}", ce.what());
        return 1;
    }
    catch (const std::runtime_error &re)
    {
        spdlog::error("runtime error: {}", re.what());
        return 1;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Unexpected error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}