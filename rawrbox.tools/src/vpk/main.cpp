#include <rawrbox/tools/vpk/packer.hpp>
#include <rawrbox/utils/crc.hpp>
#include <rawrbox/utils/md5.hpp>
#include <rawrbox/utils/vpk.hpp>

#include <fmt/format.h>

#include <bit>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace {
	void printHelp() {
		fmt::print(
		    "HELP:\n"
		    "> rawrbox_vpk pack <output_dir.vpk> --input <prefix>=<folder> [options]\n"
		    "> rawrbox_vpk list <file_dir.vpk>\n"
		    "> rawrbox_vpk verify <file_dir.vpk>\n"
		    "> rawrbox_vpk extract <file_dir.vpk> <output folder>\n"
		    "\n\n"
		    "OPTIONS:\n"
		    "  --input <prefix>=<folder> Pack a folder with a prefix\n"
		    "  --compress                Compress textures & models\n");
	}

	int pack(const std::vector<std::string>& args) {
		rawrbox::VPKPackSettings settings = {};

		const std::map<std::string, std::vector<std::string>*> globs = {
		    {"--raw", &settings.raw},
		    {"--data", &settings.data},
		    {"--preload", &settings.preload},
		};

		std::vector<std::string> positional = {};
		for (size_t i = 0; i < args.size(); i++) {
			const auto& arg = args[i];
			const auto value = [&args, &arg, &i]() -> std::string {
				if (i + 1 >= args.size()) RAWRBOX_CRITICAL("Missing value for '{}'", arg);
				return args[++i];
			};

			if (globs.contains(arg)) {
				globs.at(arg)->push_back(value());
			} else if (arg == "--input") {
				const auto input = value();
				const auto split = input.find('=');

				if (split == std::string::npos) {
					settings.inputs.emplace_back("", input);
				} else {
					settings.inputs.emplace_back(input.substr(0, split), input.substr(split + 1));
				}
			} else if (arg == "--cache") {
				settings.cache = value();
			} else if (arg == "--compress") {
				settings.compress = true;
			} else if (arg.starts_with("--")) {
				RAWRBOX_CRITICAL("Unknown option '{}'", arg);
			} else {
				positional.push_back(arg);
			}
		}

		if (positional.size() != 1) {
			printHelp();
			return 1;
		}

		settings.output = positional[0];

		if (settings.cache.empty()) {
			const auto output = std::filesystem::weakly_canonical(settings.output).generic_string();

			auto temp = std::filesystem::temp_directory_path().generic_string();
			if (temp.ends_with('/')) temp.pop_back();

			settings.cache = fmt::format("{}/rawrbox_vpk/{}", temp, rawrbox::MD5::toString(rawrbox::MD5::calculate(std::bit_cast<const uint8_t*>(output.data()), output.size())));
		}

		rawrbox::VPKPacker packer(settings);
		packer.pack();

		return 0;
	}

	int list(const std::filesystem::path& path) {
		const rawrbox::VPK vpk = rawrbox::VPK{path};

		const std::map<std::string, rawrbox::VPKEntry> entries(vpk.getEntries().begin(), vpk.getEntries().end());
		for (const auto& entry : entries) {
			const auto location = entry.second.archive == 0x7FFF ? std::string("dir") : fmt::format("{:03}", entry.second.archive);
			fmt::print("{:<64} {:>12} bytes  {:>4}  crc {:08x}{}{}\n", entry.first, static_cast<uint64_t>(entry.second.preloadSize) + entry.second.size, location, entry.second.crc, entry.second.preloadSize > 0 ? "  (preload)" : "", entry.second.compressed ? "  (zstd)" : "");
		}

		fmt::print("{} files\n", entries.size());
		return 0;
	}

	int verify(const std::filesystem::path& path) {
		rawrbox::VPK vpk = rawrbox::VPK{path};

		size_t failed = 0;
		for (const auto& entry : vpk.getEntries()) {
			const auto data = vpk.read(entry.second);
			if (CRC::Calculate(data.data(), data.size(), CRC::CRC_32()) == entry.second.crc) continue;

			fmt::print("CRC mismatch: {}\n", entry.first);
			failed++;
		}

		fmt::print("{} files verified, {} failed\n", vpk.getEntries().size(), failed);
		return failed == 0 ? 0 : 1;
	}

	int extract(const std::filesystem::path& path, const std::filesystem::path& output) {
		rawrbox::VPK vpk = rawrbox::VPK{path};

		size_t extracted = 0;
		for (const auto& entry : vpk.getEntries()) {
			const auto relative = std::filesystem::path(entry.first).lexically_normal();
			if (relative.has_root_path() || (!relative.empty() && *relative.begin() == "..")) {
				fmt::print("Skipping unsafe path '{}'\n", entry.first);
				continue;
			}

			const auto file = fmt::format("{}/{}", output.generic_string(), relative.generic_string());
			std::filesystem::create_directories(std::filesystem::path(file).parent_path());

			const auto data = vpk.read(entry.second);
			std::ofstream stream(file, std::ios::binary | std::ios::trunc);
			stream.write(std::bit_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
			extracted++;
		}

		fmt::print("{} files extracted\n", extracted);
		return 0;
	}
} // namespace

int main(int argc, char** argv) {
	const std::vector<std::string> args(argv + 1, argv + argc);
	if (args.size() < 2) {
		printHelp();
		return 1;
	}

	try {
		const std::vector<std::string> options(args.begin() + 1, args.end());

		if (args[0] == "pack")
			return pack(options);

		if (args[0] == "list")
			return list(options[0]);

		if (args[0] == "verify")
			return verify(options[0]);

		if (args[0] == "extract" && options.size() == 2)
			return extract(options[0], options[1]);
		
	} catch (const std::exception& e) {
		fmt::print(stderr, "Failed ---> {}\n", e.what());
		return 1;
	}

	printHelp();
	return 1;
}