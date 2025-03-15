#include "Logging.h"
#include <string>
#include <chrono>
#include <format>


void Logging::InitializeLog(std::filesystem::path _path, std::string name, bool append, bool timestamp)
{
	log_directory = _path;
	if (timestamp) {
		std::stringstream ss;
		auto now = std::chrono::system_clock::now();
		ss << now;
		std::cout << ss.str() << "\t" << std::format("{:%Y-%m-%d_%H-%M-%S}", std::chrono::floor<std::chrono::seconds>(now)) << "\n";
		Profile::Init(name + "_" + ss.str());
		Log::Init(name + "_" + ss.str());
		LogUsage::Init(name + "_" + ss.str());
	} else {
		Profile::Init(name, append);
		Log::Init(name, append);
		LogUsage::Init(name, append);
	}
}
