#include <rawrbox/utils/file_watcher.hpp>

#include <stdexcept>

namespace rawrbox {
	FileWatcher::FileWatcher(const std::function<void(std::filesystem::path, rawrbox::FileStatus)>& action, std::chrono::duration<int, std::milli> delay) : _delay{delay}, _action(action) {}
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
		this->_files[path] = std::filesystem::last_write_time(path);
	}

	void FileWatcher::start() {
		// NOLINTBEGIN(cppcoreguidelines-owning-memory)
		this->_thread = new std::thread([this]() {
			while (!this->_stopThread) {
				// Wait for "delay" milliseconds
				std::this_thread::sleep_for(this->_delay);
				if (this->_files.empty()) continue;

				auto it = this->_files.begin();
				while (it != this->_files.end()) {
					if (!std::filesystem::exists(it->first)) {
						if (this->_action != nullptr) this->_action(it->first, FileStatus::erased);
						it = this->_files.erase(it);
					} else {
						auto current_file_last_write_time = std::filesystem::last_write_time((*it).first);

						if ((*it).second != current_file_last_write_time) {
							(*it).second = current_file_last_write_time;
							if (this->_action != nullptr) this->_action((*it).first, FileStatus::modified);
						}

						it++;
					}
				}
			}
		});
		// NOLINTEND(cppcoreguidelines-owning-memory)
	}
} // namespace rawrbox
