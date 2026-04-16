#include "parser.h"
#include "video_processor.h"

#include <filesystem>

namespace app_parser
{
	cinebar_types::InputArgs ParseArgs(int argc, char **argv)
	{
		CLI::App app("CineBar CLI - Movie Barcode Generator");
		cinebar_types::InputArgs args;

		app.add_flag("-v,--version", args.show_info, "Display application version info");
		app.add_option("input", args.input_video_path, "Input video file")->check(CLI::ExistingFile);
		app.add_option("-o,--output", args.output_img_path, "Output barcode image filename. If not provided, a name will be automatically generated.");
		auto opt_interval = app.add_option("-i,--interval", args.interval, "Frame sampling interval in seconds")->check(CLI::PositiveNumber);
		auto opt_frames = app.add_option("-n,--frames", args.nframes, "Number of frames to sample in the visualization")->check(CLI::PositiveNumber);
		app.add_option("-m,--method", args.method, "Color extraction method")
			->transform(CLI::CheckedTransformer(cinebar_types::kArgMethodMap, CLI::ignore_case))
			->description("Method: avg | smoothed | kmeans | hist | hsv");
		app.add_option("-t,--type", args.type, "Barcode type")
			->transform(CLI::CheckedTransformer(cinebar_types::kArgTypeMap, CLI::ignore_case))
			->description("Type: color (1 color per frame) | stripe (frame slice)");
		auto bar_w = app.add_option("-w,--bar-width", args.bar_w, "Width of each barcode stripe in pixels")->check(CLI::PositiveNumber);
		app.add_option("-H,--height", args.height, "Height of the output image in pixels")->check(CLI::PositiveNumber);
		auto opt_circular = app.add_flag("-c,--circular", [&](std::int64_t)
										 { args.shape = cinebar_types::Shape::Circular; });
		app.add_flag("-r,--trim", args.trim, "Trim letterboxing and end credits from the video");

		app.parse(argc, argv);

		opt_interval->excludes(opt_frames);
		opt_frames->excludes(opt_interval);
		bar_w->excludes(opt_circular);
		opt_circular->excludes(bar_w);

		if (args.type == cinebar_types::Type::Stripe && args.shape == cinebar_types::Shape::Circular)
			throw CLI::ValidationError("parser: Circular shape is not supported for stripe barcodes");
		if (args.type == cinebar_types::Type::Stripe &&
			(args.method == cinebar_types::Method::KMeans || args.method == cinebar_types::Method::Hist || args.method == cinebar_types::Method::HSV))
			throw CLI::ValidationError("parser: KMeans, Hist, and HSV methods are not supported for stripe barcodes");

		if (args.start_frame > 0 || args.end_frame > 0)
		{
			if (args.end_frame < args.start_frame)
				throw CLI::ValidationError("parser: End frame cannot be less than start frame");

			args.segment_nframes = args.end_frame - args.start_frame + 1;

			if (args.nframes > args.segment_nframes)
				throw CLI::ValidationError("parser: Number of frames to sample cannot exceed the total number of frames in the specified segment");
		}
		else
		{
			args.segment_nframes = 0; // Will be set to total frame count later if not specified
		}

		if (!args.show_info && args.input_video_path.empty())
			throw CLI::RequiredError("Input video");
		if (args.output_img_path.empty())
			args.output_img_path = std::filesystem::path(args.input_video_path).replace_extension(".png").string();

		return args;
	}

	void ProcessingArgs(cinebar_types::InputArgs &args, cinebar_types::VideoInfo &video_info)
	{
		if (args.segment_nframes == 0)
		{
			args.segment_nframes = video_info.frame_count; // Default to total frame count if not set by start/end frame
		}
		if (args.interval > 0.0)
		{
			args.nframes = app_video_processor::NframesFromInterval(args.segment_nframes, args.interval, video_info.fps);
		}
		else if (args.nframes > 0)
		{
			// Compute sampling FPS from desired number of frames
			double duration = static_cast<double>(args.segment_nframes) / video_info.fps;
			args.interval = args.nframes / duration;
		}
		else
		{
			// Sample all frames in the segment (if specified) or the entire video if no interval or nframes is specified
			args.nframes = args.segment_nframes;
			args.interval = 1 / video_info.fps;
		}
		// Use original height if not specified for horizontal barcodes
		if (args.shape == cinebar_types::Shape::Horizontal && args.height <= 0)
		{
			args.height = video_info.height;
			args.width = args.bar_w * args.nframes;
		}
		else
		{
			args.height = args.width = video_info.width;
		}

		if (args.end_frame == 0)
			args.end_frame = video_info.frame_count - 1; // Ensure end_frame is set to the last frame if it was 0
	}
}
