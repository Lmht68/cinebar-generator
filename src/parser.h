#ifndef PARSER_H_
#define PARSER_H_

#include "types.h"

#include <CLI/CLI.hpp>

/**
 * @file parser.h
 * @brief Command-line parsing helpers.
 */

namespace app_parser
{
	/**
	 * @brief Parses command-line options into the shared input argument structure.
	 *
	 * @param argc Number of command-line arguments.
	 * @param argv Command-line argument array.
	 * @return Parsed argument values.
	 *
	 * @throws CLI::Error Thrown when arguments are invalid or incomplete.
	 */
	cinebar_types::InputArgs ParseArgs(int argc, char **argv);

	/**
	 * @brief Derives sampling and output dimensions from loaded video metadata.
	 *
	 * @param args Parsed input arguments to normalize in place.
	 * @param video_info Loaded video metadata used to fill derived values.
	 */
	void ProcessingArgs(cinebar_types::InputArgs &args, cinebar_types::VideoInfo &video_info);
}

#endif
