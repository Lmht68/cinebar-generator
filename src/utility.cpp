#include "utility.h"

#include <indicators/cursor_control.hpp>

namespace app_utility
{
    BlockProgressBar CreateProgressBar(const std::string &prefix, size_t max_progress)
    {
        show_console_cursor(false);
        return BlockProgressBar{
            option::PrefixText{prefix},
            option::BarWidth{80},
            option::ForegroundColor{Color::yellow},
            option::ShowPercentage{true},
            option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
            option::MaxProgress{max_progress}};
    }

    void UpdateProgressBar(BlockProgressBar &bar, size_t current, size_t total)
    {
        bar.tick();
        bar.set_option(option::PostfixText{std::to_string(current) + "/" + std::to_string(total)});

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

    ProgressSpinner CreateProgressSpinner(const std::string &postfix)
    {
        show_console_cursor(false);
        return ProgressSpinner{
            option::PostfixText{postfix},
            option::ForegroundColor{Color::yellow},
            option::ShowPercentage{false},
            option::SpinnerStates{std::vector<std::string>{"▖", "▘", "▝", "▗"}},
            option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}};
    }

    std::thread StartSpinnerJob(ProgressSpinner &spinner)
    {
        show_console_cursor(false);
        return std::thread([&spinner]()
                           {
            while (!spinner.is_completed())
            {
                spinner.tick();
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
            } });
    }

    void StopSpinnerJob(ProgressSpinner &spinner, std::thread &spinner_thread)
    {
        spinner.set_option(option::ForegroundColor{Color::green});
        spinner.set_option(option::ShowSpinner{false});
        spinner.mark_as_completed();
        show_console_cursor(true);
        spinner_thread.join();
    }
}