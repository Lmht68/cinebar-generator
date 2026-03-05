#ifndef PARSER_H_
#define PARSER_H_

#include <CLI/CLI.hpp>

#include <string>

namespace app_parser
{
	struct InputArgs
	{
		std::string input_video_path;
		std::string output_img_path;
		double interval = 0.0;
		int nframes = 0;
		int width = 0;
		int height = 0;
		bool show_info = false;
	};

	InputArgs ParseArgs(int argc, char **argv);
}

#endif