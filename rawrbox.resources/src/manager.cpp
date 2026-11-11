#include <rawrbox/resources/loaders/json.hpp>
#include <rawrbox/resources/manager.hpp>

namespace rawrbox {
	std::mutex rawrbox::RESOURCES::_threadLock;

	std::unordered_set<std::string> rawrbox::RESOURCES::_loadedFiles = {};

	// PRELOAD ---
	std::mutex rawrbox::RESOURCES::_preloadLock;
	std::atomic<size_t> rawrbox::RESOURCES::_preloadLoaded = 0;
	std::atomic<size_t> rawrbox::RESOURCES::_preloadTotal = 0;
	// -----------

	thread_local std::vector<std::string> rawrbox::RESOURCES::_loading = {};

	std::vector<std::unique_ptr<rawrbox::Loader>> rawrbox::RESOURCES::_loaders = [] {
		std::vector<std::unique_ptr<rawrbox::Loader>> defaults;
		defaults.push_back(std::make_unique<rawrbox::JSONLoader>());
		return defaults;
	}();

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> rawrbox::RESOURCES::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Resources");
	// -------------
} // namespace rawrbox