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

	inline const char *ToString(BarcodeShape shape)
	{
		switch (shape)
		{
		case BarcodeShape::Horizontal:
			return "Horizontal";
		case BarcodeShape::Circular:
			return "Circular";
		default:
			return "Unknown";
		}
	}

	const std::map<std::string, BarcodeShape> kArgShapeMap{
		{"horizontal", BarcodeShape::Horizontal},
		{"circular", BarcodeShape::Circular}};

	enum class Method
	{
		Avg,
		Smoothed,
		KMeans,
		HSV,
		Stripe,
	};

	inline const char *ToString(Method method)
	{
		switch (method)
		{
		case Method::Avg:
			return "Average";
		case Method::Smoothed:
			return "Smoothed";
		case Method::KMeans:
			return "K-Means";
		case Method::HSV:
			return "HSV";
		case Method::Stripe:
			return "Stripe";
		default:
			return "Unknown";
		}
	}

	const std::map<std::string, app_parser::Method> kArgMethodMap{
		{"avg", app_parser::Method::Avg},
		{"smoothed", app_parser::Method::Smoothed},
		{"kmeans", app_parser::Method::KMeans},
		{"hsv", app_parser::Method::HSV},
		{"stripe", app_parser::Method::Stripe}};

	struct InputArgs
	{
		std::string input_video_path;
		std::string output_img_path;
		double interval = 0.0;
		int nframes = 0;
		int bar_w = 1;
		int width = 0;
		int height = 0;
		// TODO: start and end frames are for credit/end trimming
		int start_frame = 0;
		int end_frame = -1; // -1 means till the end of the video
		BarcodeShape shape = BarcodeShape::Horizontal;
		bool trim = false;
		bool show_info = false;
		Method method = Method::Avg;
		int workers = 1;
	};

	InputArgs ParseArgs(int argc, char **argv);
}

#endif