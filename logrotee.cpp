#include <iostream>
#include <string>
#include <getopt.h>

#pragma ide diagnostic ignored "ClangTidyInspection"

using std::string;

const char *programName = "0.1";
const char *programVersion = "0.1";


struct Arguments {
	bool invalid;
	string logFilePath;
	string compressCommand;
	string compressSuffix;
	bool nullStdout;
	size_t chunkSize;
	bool dates;
	ssize_t maxFiles;
	
	bool isInvalid() const {
		return invalid || logFilePath.empty();
	}
} commandArgs;

void usage() {
	std::cout << "Example usage: verbosecommand | logrotee /var/log/verbosecommand.log"
			" --compress \"bzip2 {}\" --compress-suffix .bz2"
			" --null --chunk 2M" << std::endl;
}


int main(int argc, char *argv[])
{
	int getoptResult;
	
	for(;;) {
		int optionIndex = 0;
		static struct option longOptions[] = {
				{"compress", required_argument, nullptr, 'c' },
				{"compress-suffix", required_argument, nullptr, 's' },
				{"null", no_argument, nullptr, 'n' },
				{"dates", no_argument, nullptr, 'd' },
				{"max-files", required_argument, nullptr, 'm'},
				{"help", no_argument, nullptr, 'h' },
				{"version", no_argument, nullptr, 'v' },
				{nullptr, 0, nullptr, 0 }
		};
		
		getoptResult = getopt_long(argc, argv, "", longOptions, &optionIndex);
		if (getoptResult == -1)
			break;
		
		switch (getoptResult) {
			case 'c':
				commandArgs.compressCommand = optarg;
				break;
			
			case 's':
				commandArgs.compressSuffix = optarg;
				break;

			case 'n':
				commandArgs.nullStdout = true;
				break;

			case 'd':
				commandArgs.dates = true;
				break;

			case 'm':
				try {
					commandArgs.maxFiles = std::stoi(optarg);
				}
				catch (const std::logic_error& e) {
					std::cout << "Cannot parse number: " << optarg << std::endl;
					exit(1);
				}
				break;

			case 'h':
				usage();
				exit(0);

			case 'v':
				std::cout << programName << " " << programVersion << std::endl;
				exit(0);
			
			default:
				printf("?? getopt returned character code 0%o ??\n", getoptResult);
		}
	}
	
	if (optind < argc) {
		commandArgs.logFilePath = argv[optind];
		optind++;
		while (optind < argc) {
			commandArgs.invalid = true;
			std::cerr << "Extra command line argument:" << argv[optind] << std::endl;
		}
	}

	if (commandArgs.isInvalid()) {
		usage();
		exit(1);
	}



	return 0;
}
