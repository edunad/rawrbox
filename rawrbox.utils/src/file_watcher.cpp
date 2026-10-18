#include <rawrbox/utils/file_watcher.hpp>

#include <system_error>
#include <utility>
#include <vector>

namespace rawrbox {
	FileWatcher::FileWatcher(const std::function<void(std::string, rawrbox::FileStatus)>& action, std::chrono::duration<int, std::milli> delay) : _delay{delay}, _action(action) {}
	FileWatcher::~FileWatcher() { this->stop(); }

	void FileWatcher::stop() {
		this->_stopThread = true;

		// NOLINTBEGIN(cppcoreguidelines-owning-memory)
		if (this->_thread != nullptr) {
			if (this->_thread->joinable()) this->_thread->join();
			delete this->_thread;
			this->_thread = nullptr;
		}
		// NOLINTEND(cppcoreguidelines-owning-memory)
	}

	void FileWatcher::watchFile(const std::filesystem::path& path) {
		const auto writeTime = std::filesystem::last_write_time(path);

		const std::scoped_lock lock(this->_filesLock);
		this->_files[path.generic_string()] = writeTime;
	}

	void FileWatcher::unwatchFile(const std::filesystem::path& path) {
		const std::scoped_lock lock(this->_filesLock);

		const auto fnd = this->_files.find(path.generic_string());
		if (fnd == this->_files.end()) return;

		this->_files.erase(fnd);
	}

	void FileWatcher::start() {
		// NOLINTBEGIN(cppcoreguidelines-owning-memory)
		this->_thread = new std::thread([this]() {
			while (!this->_stopThread) {
				// Wait for "delay" milliseconds
				std::this_thread::sleep_for(this->_delay);

				std::vector<std::pair<std::string, rawrbox::FileStatus>> changes = {};

				// THREAD LOCK ---
				{
					const std::scoped_lock lock(this->_filesLock);

					auto it = this->_files.begin();
					while (it != this->_files.end()) {
						std::error_code error = {};
						const auto current_file_last_write_time = std::filesystem::last_write_time(it->first, error);

						if (error) {
							changes.emplace_back(it->first, FileStatus::erased);
							it = this->_files.erase(it);
							continue;
						}

						if (it->second != current_file_last_write_time) {
							it->second = current_file_last_write_time;
							changes.emplace_back(it->first, FileStatus::modified);
						}

						++it;
					}
				}
				// ----------------

				if (this->_action == nullptr) continue;
				for (const auto& [file, status] : changes) {
					this->_action(file, status);
				}
			}
		});
		// NOLINTEND(cppcoreguidelines-owning-memory)
	}
} // namespace rawrbox