#include <string>
#include <iostream>
#include <getopt.h>
#include <sys/stat.h>

#define BUF_SIZE 4096
#define DEFAULT_CHUNK_SIZE (10*1000*1000)

#pragma ide diagnostic ignored "ClangTidyInspection"

using std::string;

const char *programName = "logrotee";
const char *programVersion = "0.0.1";


struct Arguments {
	string logFilePath;
	string compressCommand;
	string compressSuffix;
	bool nullStdout;
	size_t chunkSize;
	bool dates;
	ssize_t maxFiles;

private:
	bool invalid;
	
public:
	Arguments(int argc, char *argv[])
			: chunkSize(DEFAULT_CHUNK_SIZE)
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
					{"chunk", required_argument, nullptr, 'k'},
					{"help", no_argument, nullptr, 'h' },
					{"version", no_argument, nullptr, 'v' },
					{nullptr, 0, nullptr, 0 }
			};
			
			getoptResult = getopt_long(argc, argv, "", longOptions, &optionIndex);
			if (getoptResult == -1)
				break;
			
			switch (getoptResult) {
				case 'c':
					compressCommand = optarg;
					break;
				
				case 's':
					compressSuffix = optarg;
					break;
				
				case 'n':
					nullStdout = true;
					break;
				
				case 'd':
					dates = true;
					break;
				
				case 'm':
					try {
						maxFiles = std::stoi(optarg);
					}
					catch (const std::logic_error& e) {
						std::cout << "Cannot parse number: " << optarg << std::endl;
						exit(EXIT_FAILURE);
					}
					break;
				
				case 'k':
					try {
						chunkSize = (size_t) std::stoi(optarg);
					}
					catch (const std::logic_error& e) {
						std::cout << "Cannot parse number: " << optarg << std::endl;
						exit(EXIT_FAILURE);
					}
					break;
				
				case 'h':
					usage();
					exit(EXIT_SUCCESS);
				
				case 'v':
					std::cout << programName << " " << programVersion << std::endl;
					exit(EXIT_SUCCESS);
				
				default:
					printf("getopt returned unexpected character 0x%x\n", getoptResult);
			}
		}
		
		if (optind < argc) {
			logFilePath = argv[optind];
			optind++;
			while (optind < argc) {
				invalid = true;
				std::cerr << "Extra command line argument:" << argv[optind] << std::endl;
				optind++;
			}
		}
	}
	
	bool isInvalid() const {
		return invalid || logFilePath.empty();
	}
	
	void usage() {
		std::cout << "Example usage: verbose_command | logrotee /var/log/verbose_command.log"
				" --compress \"bzip2 {}\" --compress-suffix .bz2"
				" --null --chunk 2M" << std::endl;
	}
};


struct Logrotatee {
	pid_t lastChild;
	FILE *chunkFile;
	size_t bytesInChunk;
	const Arguments& commandArgs;
	
	Logrotatee(const Arguments& args) : lastChild(0), chunkFile(nullptr), bytesInChunk(0), commandArgs(args) {};
	void go();
	
private:
	void rotateLog();
	string getNewChunkName();
};


bool fileExists(string name) {
	struct stat sb;
	return stat(name.c_str(), &sb) == 0;
}

string Logrotatee::getNewChunkName() {
	int suffix = 0;
	string name;
	do {
		suffix++;
		name = commandArgs.logFilePath + '.' + std::to_string(suffix);
	} while (fileExists(name + commandArgs.compressSuffix));
	
	return name;
}

string replaceSubstring(string str, const string& what, const string& with) {
	size_t startPos = str.find(what);
	if(startPos == string::npos)
		return str;
	str.replace(startPos, what.length(), with);
	return str;
}

int execCompression(string command) {
	
	static pid_t lastChild = 0;
	
	if (lastChild != 0) {
		siginfo_t siginfo;
		siginfo.si_pid = 0;
		int childStatusChanged = waitid(P_PID, lastChild, &siginfo, WNOHANG);
		if (childStatusChanged != 0) {
			fprintf(stderr, "Error in waitid()\n");
		} else if (siginfo.si_pid != 0) {
			fprintf(stderr, "Warning: a previous compression process %d==%d has not exited yet.\n", siginfo.si_pid, lastChild);
		}
	}
	
	pid_t child = fork();
	if (child > 0) {
		lastChild = child;
	} else if (child < 0) {
		fprintf(stderr, "Error in fork()\n");
	} else {
		int status = system(command.c_str());
		if (WEXITSTATUS(status) != 0) {
			fprintf(stderr, "Error %d running the command(%s)\n", WEXITSTATUS(status), command.c_str());
			return 2;
		}
	}
	
	return 0;
}

void Logrotatee::rotateLog() {
	const char *logFilePath = commandArgs.logFilePath.c_str();
	
	if (chunkFile != nullptr) {
		string newName = getNewChunkName();
		fclose(chunkFile);
		chunkFile = nullptr;
		rename(logFilePath, newName.c_str());
		
		if (!commandArgs.compressCommand.empty()) {
			string command = replaceSubstring(commandArgs.compressCommand, "{}", newName);
			execCompression(command);
		}
	}
	
	if (fopen(logFilePath, "a") < 0) {
		fprintf(stderr, "Error opening %s: %d (%s)\n", logFilePath, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	chunkFile = fopen(logFilePath, "a");
}

void Logrotatee::go() {
	char buffer[BUF_SIZE];
	
	rotateLog();
	for(char *s = fgets(buffer, BUF_SIZE, stdin); s != nullptr; s = fgets(buffer, BUF_SIZE, stdin)) {
		fputs(s, chunkFile);
		if (!commandArgs.nullStdout) {
			fputs(s, stdout);
		}
		
		// As much as we want to avoid extra scanning, we need to know when to rotate logs.
		size_t length = strlen(s);
		bytesInChunk += length;
		if (length > 0 && s[length-1] == '\n' && bytesInChunk >= commandArgs.chunkSize) {
			rotateLog();
			bytesInChunk = 0;
		}
	}
	
	fclose(chunkFile);
	chunkFile = nullptr;
}

int main(int argc, char *argv[])
{
	Arguments commandArgs(argc, argv);

	if (commandArgs.isInvalid()) {
		commandArgs.usage();
		exit(EXIT_FAILURE);
	}
	
	Logrotatee lr(commandArgs);
	lr.go();

	return 0;
}
