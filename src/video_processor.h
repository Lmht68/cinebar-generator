#ifndef VIDEO_PROCESSOR_H_
#define VIDEO_PROCESSOR_H_

#include "types.h"
#include "parser.h"
#include "frame_extractor.h"

#include <opencv2/videoio.hpp>

#include <atomic>

/**
 * @file video_processor.h
 * @brief Video loading, sampling, crop detection, and extraction dispatch helpers.
 */

namespace app_video_processor
{
	/** @brief Downscale factor used during black-bar detection. */
	inline constexpr double kDownScaleFactor = 0.25; // 1/4 size for faster processing
	/** @brief Minimum retained size ratio before a crop is ignored as insignificant. */
	inline constexpr double kMinCropRatio = 0.97;	 // Ignore crops that keep more than 97% of the original frame
	/** @brief Pixel intensity threshold used when classifying black bars. */
	inline constexpr int kDefaultThreshold = 16;
	/** @brief Required black-pixel ratio for a row or column to count as boxed. */
	inline constexpr double kDefaultMinBlackRatio = 0.98;
	/** @brief Number of frames sampled while inferring visible-content bounds. */
	inline constexpr int kDefaultSampleFrames = 10;

	/**
	 * @brief Opens a video file and gathers its metadata.
	 *
	 * @param video_path Path to the source video file.
	 * @return Loaded video metadata and an initialized capture handle.
	 */
	cinebar_types::VideoInfo LoadVideoInfo(const std::string &video_path);

	/**
	 * @brief Computes how many samples fit in a frame range for a given interval.
	 *
	 * @param frame_count Number of frames available in the sampled segment.
	 * @param interval Sampling interval in seconds.
	 * @param fps Source video frame rate in frames per second.
	 * @return Number of frames that will be sampled.
	 */
	int NframesFromInterval(const int frame_count,
							const double interval,
							const double fps);

	/**
	 * @brief Detects the non-black content bounds within a grayscale frame.
	 *
	 * @param frame_grayed Grayscale frame used for black-bar analysis.
	 * @return Visible-content bounds when black bars are detected, otherwise `std::nullopt`.
	 */
	std::optional<cinebar_types::VideoBounds> DetectBounds(const cv::Mat &frame_grayed);

	/**
	 * @brief Crops a frame in place to the provided visible-content bounds.
	 *
	 * @param frame Frame to crop.
	 * @param bounds Detected bounds to keep.
	 */
	void CropImage(cv::Mat &frame,
				   const cinebar_types::VideoBounds &bounds);

	/**
	 * @brief Detects letterboxing or pillarboxing and updates the video metadata.
	 *
	 * @param video_info Loaded video metadata updated in place.
	 */
	void DetectVideoBoxType(cinebar_types::VideoInfo &video_info);

	/**
	 * @brief Extracts one representative color per sampled frame.
	 *
	 * @tparam Extractor Callable used to reduce each frame to a `cv::Vec3b`.
	 * @param args Sampling and trim settings.
	 * @param video_info Loaded video metadata and capture handle.
	 * @param progress_current Atomic progress counter updated after each sample.
	 * @return Sequence of colors in sampling order.
	 */
	template <auto Extractor>
	std::vector<cv::Vec3b> ExtractColors(const cinebar_types::InputArgs &args,
										 cinebar_types::VideoInfo &video_info,
										 std::atomic<int> &progress_current);

	/**
	 * @brief Extracts one representative stripe image per sampled frame.
	 *
	 * @tparam Extractor Callable used to reduce each frame to a `cv::Mat` stripe.
	 * @param args Sampling and trim settings.
	 * @param video_info Loaded video metadata and capture handle.
	 * @param progress_current Atomic progress counter updated after each sample.
	 * @return Sequence of stripe images in sampling order.
	 */
	template <auto Extractor>
	std::vector<cv::Mat> ExtractStripes(const cinebar_types::InputArgs &args,
										cinebar_types::VideoInfo &video_info,
										std::atomic<int> &progress_current);

	/**
	 * @brief Dispatches color extraction to the algorithm selected in `args.method`.
	 *
	 * @param args Sampling and extraction settings.
	 * @param video_info Loaded video metadata and capture handle.
	 * @param progress_current Atomic progress counter updated after each sample.
	 * @return Sequence of extracted colors.
	 */
	std::vector<cv::Vec3b> ExtractColorsDispatch(const cinebar_types::InputArgs &args,
												 cinebar_types::VideoInfo &video_info,
												 std::atomic<int> &progress_current);

	/**
	 * @brief Dispatches stripe extraction to the algorithm selected in `args.method`.
	 *
	 * @param args Sampling and extraction settings.
	 * @param video_info Loaded video metadata and capture handle.
	 * @param progress_current Atomic progress counter updated after each sample.
	 * @return Sequence of extracted stripe images.
	 */
	std::vector<cv::Mat> ExtractStripesDispatch(const cinebar_types::InputArgs &args,
												cinebar_types::VideoInfo &video_info,
												std::atomic<int> &progress_current);
}

#endif
