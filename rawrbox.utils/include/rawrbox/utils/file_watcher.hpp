#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>

namespace rawrbox {
	enum class FileStatus {
		modified,
		erased
	};

	class FileWatcher {
		bool _stopThread = false;
		std::thread* _thread = nullptr;
		std::chrono::duration<int, std::milli> _delay;

		std::function<void(std::string, rawrbox::FileStatus)> _action = nullptr;
		std::unordered_map<std::string, std::filesystem::file_time_type> _files = {};

	public:
		FileWatcher(const FileWatcher&) = default;
		FileWatcher(FileWatcher&&) = delete;
		FileWatcher& operator=(const FileWatcher&) = default;
		FileWatcher& operator=(FileWatcher&&) = delete;
		FileWatcher(const std::function<void(std::string, rawrbox::FileStatus)>& action, std::chrono::duration<int, std::milli> delay);
		~FileWatcher();

		void watchFile(const std::filesystem::path& path);
		void unwatchFile(const std::filesystem::path& path);

		void stop();
		void start();
	};
} // namespace rawrbox
