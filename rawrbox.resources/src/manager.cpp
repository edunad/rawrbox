#include <rawrbox/resources/loaders/json.hpp>
#include <rawrbox/resources/manager.hpp>

namespace rawrbox {
	std::mutex rawrbox::RESOURCES::_threadLock;

	std::vector<std::filesystem::path> rawrbox::RESOURCES::_loadedFiles = {};
	std::atomic<size_t> rawrbox::RESOURCES::_loadingFiles = 0;

	std::vector<std::unique_ptr<rawrbox::Loader>> rawrbox::RESOURCES::_loaders = [] {
		std::vector<std::unique_ptr<rawrbox::Loader>> defaults;
		defaults.push_back(std::make_unique<rawrbox::JSONLoader>());
		return defaults;
	}();

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> rawrbox::RESOURCES::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Resources");
	// -------------
} // namespace rawrbox
