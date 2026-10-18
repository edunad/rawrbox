#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace rawrbox {
	enum class FileStatus {
		modified,
		erased
	};

	class FileWatcher {
		std::atomic<bool> _stopThread = false;
		std::thread* _thread = nullptr;
		std::chrono::duration<int, std::milli> _delay;

		std::function<void(std::string, rawrbox::FileStatus)> _action = nullptr;
		std::unordered_map<std::string, std::filesystem::file_time_type> _files = {};

		std::mutex _filesLock = {};

	public:
		FileWatcher(const FileWatcher&) = delete;
		FileWatcher(FileWatcher&&) = delete;
		FileWatcher& operator=(const FileWatcher&) = delete;
		FileWatcher& operator=(FileWatcher&&) = delete;
		FileWatcher(const std::function<void(std::string, rawrbox::FileStatus)>& action, std::chrono::duration<int, std::milli> delay);
		~FileWatcher();

		void watchFile(const std::filesystem::path& path);
		void unwatchFile(const std::filesystem::path& path);

		void stop();
		void start();
	};
} // namespace rawrbox