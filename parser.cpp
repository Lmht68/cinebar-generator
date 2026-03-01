#include "parser.h"

namespace app_parser {
	InputArgs ParseArgs(int argc, char** argv) {
		CLI::App app("CineBar CLI - Movie Barcode Generator");

		InputArgs args;
		
		app.add_option("input", args.input_video_path, "Input video file")->check(CLI::ExistingFile);
		app.add_option("-o,--output", args.output_img_path, "Output barcode image filename. If not provided, a name will be automatically generated.");
		app.add_flag("-v,--version", args.show_info, "Display application version info");

		try
		{
			app.parse(argc, argv);
		}
		catch (const CLI::ParseError& e)
		{
			std::exit(app.exit(e));
		}

		if (!args.show_info && args.input_video_path.empty()) {
			throw CLI::RequiredError("Input video");
		}

		return args;
	}
}