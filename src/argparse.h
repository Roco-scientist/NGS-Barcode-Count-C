#ifndef argparse
#define argparse

#include <string>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

struct Args {
	std::string sample_barcodes_file;
	std::string counted_barcodes_file;

	void get_args(int argc, char argv[]);
};
#endif
