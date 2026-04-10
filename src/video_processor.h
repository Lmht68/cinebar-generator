#ifndef VIDEO_PROCESSOR_H_
#define VIDEO_PROCESSOR_H_

#include "types.h"
#include "parser.h"
#include "frame_extractor.h"
#include "utility.h"

#include <opencv2/videoio.hpp>

namespace app_video_processor
{
	constexpr double kDownScaleFactor = 0.25; // 1/4 size for faster processing
	constexpr double kMinCropRatio = 0.97;	  // Ignore crops that keep more than 97% of the original frame
	constexpr int kDefaultThreshold = 16;
	constexpr double kDefaultMinBlackRatio = 0.98;
	constexpr int kDefaultSampleFrames = 10;

	cinebar_types::VideoInfo LoadVideoInfo(const std::string &video_path);
	size_t NframesFromInterval(const size_t frame_count,
							   const double interval,
							   const double fps);
	std::optional<cinebar_types::VideoBounds> DetectBounds(const cv::Mat &frame_grayed);
	void CropImage(cv::Mat &frame,
				   const cinebar_types::VideoBounds &bounds);
	void DetectVideoBoxType(cinebar_types::VideoInfo &video_info,
							ProgressCbk on_start = nullptr,
							ProgressCbk on_finish = nullptr);

	std::vector<cv::Vec3b> ExtractColors(const cinebar_types::InputArgs &args,
										 cinebar_types::VideoInfo &video_info,
										 const app_frame_extractor::ColorFunc &extractor,
										 ProgressUpdateCbk on_progress = nullptr,
										 ProgressCbk on_cancel = nullptr);
	std::vector<cv::Mat> ExtractStripes(const cinebar_types::InputArgs &args,
										cinebar_types::VideoInfo &video_info,
										ProgressUpdateCbk on_progress = nullptr,
										ProgressCbk on_cancel = nullptr);
}

#endif