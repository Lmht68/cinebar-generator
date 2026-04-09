#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <indicators/progress_spinner.hpp>
#include <indicators/block_progress_bar.hpp>

#include <string>

namespace app_utility
{
    using namespace indicators;

    BlockProgressBar CreateProgressBar(const std::string &prefix, size_t max_progress);
    void UpdateProgressBar(BlockProgressBar &bar, size_t current, size_t total);
    void CancelProgressBar(BlockProgressBar &bar);

    ProgressSpinner CreateProgressSpinner(const std::string &prefix);
    std::thread StartSpinnerJob(ProgressSpinner &spinner);
    void StopSpinnerJob(ProgressSpinner &spinner, std::thread &spinner_thread);
}

#endif