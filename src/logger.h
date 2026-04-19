#ifndef LOGGER_H_
#define LOGGER_H_

#include "spdlog/spdlog.h"

/**
 * @file logger.h
 * @brief Logging initialization.
 */

namespace app_logger
{
    /**
     * @brief Initializes the application logger and OpenCV logging bridge.
     *
     * @throws std::runtime_error Thrown when logger setup fails.
     */
    void InitLogger();

    /**
     * @brief Returns the lazily created logger used for forwarded OpenCV messages.
     *
     * @return Reference to the shared OpenCV logger pointer.
     */
    std::shared_ptr<spdlog::logger> &CvLogger();
}

#endif
