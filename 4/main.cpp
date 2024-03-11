#include "Logger.h"

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cout << "Usage: \"COM_PORT_HERE\"" << std::endl;
		return 1;
	}
	Logger::OpenPort(argv[1]);
	Logger::StartLogging();

	return 0;
}
