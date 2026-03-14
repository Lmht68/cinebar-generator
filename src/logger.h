#ifndef LOGGER_H_
#define LOGGER_H_

#include "spdlog/spdlog.h"

namespace app_logger
{
    void InitLogger();
    std::shared_ptr<spdlog::logger> &CvLogger();
}

#endif