#ifndef PARSER_H_
#define PARSER_H_

#include "types.h"

#include <CLI/CLI.hpp>

namespace app_parser
{
	cinebar_types::InputArgs ParseArgs(int argc, char **argv);
}

#endif