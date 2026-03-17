#ifndef PARSER_H_
#define PARSER_H_

#include <CLI/CLI.hpp>

#include <string>

namespace app_parser
{
	enum class BarcodeShape
	{
		Horizontal,
		Circular
	};

	inline const char *ToString(app_parser::BarcodeShape shape)
	{
		switch (shape)
		{
		case app_parser::BarcodeShape::Horizontal:
			return "Horizontal";
		case app_parser::BarcodeShape::Circular:
			return "Circular";
		default:
			return "Unknown";
		}
	}

	const std::map<std::string, app_parser::BarcodeShape> kArgShapeMap{
		{"horizontal", app_parser::BarcodeShape::Horizontal},
		{"circular", app_parser::BarcodeShape::Circular}};

	struct InputArgs
	{
		std::string input_video_path;
		std::string output_img_path;
		double interval = 0.0;
		int nframes = 0;
		int bar_w = 1;
		int width = 0;
		int height = 0;
		BarcodeShape shape = BarcodeShape::Horizontal;
		bool trim = false;
		bool show_info = false;
	};

	InputArgs ParseArgs(int argc, char **argv);
}

#endif