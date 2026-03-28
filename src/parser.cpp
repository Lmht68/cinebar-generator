#include "parser.h"

#include <filesystem>

namespace app_parser
{
	InputArgs ParseArgs(int argc, char **argv)
	{
		CLI::App app("CineBar CLI - Movie Barcode Generator");
		InputArgs args;

		app.add_flag("-v,--version", args.show_info, "Display application version info");
		app.add_option("input", args.input_video_path, "Input video file")->check(CLI::ExistingFile);
		app.add_option("-o,--output", args.output_img_path, "Output barcode image filename. If not provided, a name will be automatically generated.");
		auto interval_opt = app.add_option("-i,--interval", args.interval, "Frame sampling interval in seconds")->check(CLI::PositiveNumber);
		auto nframes_opt = app.add_option("-n,--frames", args.nframes, "Number of frames to sample in the visualization")->check(CLI::PositiveNumber);
		app.add_option("-m,--method", args.method, "Color extraction method")
			->transform(CLI::CheckedTransformer(kArgMethodMap))
			->description("Method: avg | smoothed | kmeans | hsv | stripe");
		app.add_option("-W,--bar-width", args.bar_w, "Width of each barcode stripe in the output barcode image, in pixels")->check(CLI::PositiveNumber);
		app.add_option("-H,--height", args.height, "Height of the output barcode image, in pixels")->check(CLI::PositiveNumber);
		app.add_option("-w,--workers", args.workers, "Number of worker threads to use")->check(CLI::PositiveNumber);
		app.add_flag("-c, --circular", [&](std::int64_t)
					 { args.shape = app_parser::BarcodeShape::Circular; });
		app.add_flag("-t,--trim", args.trim, "Trim letterboxing and end credits from the video");

		app.parse(argc, argv);

		bool interval_set = interval_opt->count() > 0;
		bool nframes_set = nframes_opt->count() > 0;

		if (interval_set && nframes_set)
		{
			throw CLI::ValidationError(
				"parser: Sampling options conflict: "
				"choose either --interval (time-based) "
				"or --nframes (fixed-count), but not both.");
		}

		if (!args.show_info && args.input_video_path.empty())
		{
			throw CLI::RequiredError("Input video");
		}

		if (args.output_img_path.empty())
		{
			args.output_img_path = std::filesystem::path(args.input_video_path)
									   .replace_extension(".png")
									   .string();
		}

		return args;
	}
}