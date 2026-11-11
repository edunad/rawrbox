#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace rawrbox {
	class VPKTool {
	protected:
		static void printHelp();
		static int pack(const std::vector<std::string>& args);
		static int list(const std::filesystem::path& path);
		static int verify(const std::filesystem::path& path);
		static int extract(const std::filesystem::path& path, const std::filesystem::path& output);

	public:
		static int run(const std::vector<std::string>& args);
	};
} // namespace rawrbox
