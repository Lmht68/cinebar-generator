#ifndef VIDEO_PROCESSOR_H_
#define VIDEO_PROCESSOR_H_

#include <opencv2/videoio.hpp>

#include <string>
#include <vector>
#include <optional>

namespace app_video_processor
{
	// Represents the detected bounds of the actual video content, excluding letterboxing
	struct VideoBounds
	{
		int left;
		int top;
		int right;
		int bottom;
	};
	enum class BoxType
	{
		None,
		Letterbox,
		Pillarbox,
		Windowbox
	};
	struct VideoInfo
	{
		cv::VideoCapture capture;
		double fps;
		double duration;
		std::uintmax_t size;
		int frame_count;
		int width;
		int height;
		std::optional<VideoBounds> bounds;
		BoxType box_type = BoxType::None;
	};

	constexpr double kDownScaleFactor = 0.25; // 1/4 size for faster processing
	constexpr double kMinCropRatio = 0.97;	  // Ignore crops that keep more than 97% of the original frame
	constexpr int kDefaultThreshold = 16;
	constexpr double kDefaultMinBlackRatio = 0.98;
	constexpr int kDefaultSampleFrames = 10;

	inline const char *ToString(app_video_processor::BoxType type)
	{
		switch (type)
		{
		case app_video_processor::BoxType::None:
			return "No letterboxing or pillarboxing detected";
		case app_video_processor::BoxType::Letterbox:
			return "Letterboxing detected";
		case app_video_processor::BoxType::Pillarbox:
			return "Pillarboxing detected";
		case app_video_processor::BoxType::Windowbox:
			return "Windowboxing detected";
		default:
			return "Unknown";
		}
	}

	VideoInfo LoadVideoInfo(const std::string &video_path);
	int GetFrameCountFromInterval(int frame_count, double fps, double interval);
	int NframesFromInterval(const VideoInfo &video_info, double interval);

	std::optional<VideoBounds> DetectBounds(const cv::Mat &frame_grayed);
	cv::Mat CropImage(const cv::Mat &frame, const VideoBounds &bounds);
	bool DetermineVideoBounds(VideoInfo &video_info,
							  VideoBounds &bounds);
	void DetectVideoBoxType(VideoInfo &video_info);
}

#endif