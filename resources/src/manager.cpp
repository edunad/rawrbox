#include <rawrbox/resources/loaders/json.hpp>
#include <rawrbox/resources/manager.hpp>

namespace rawrbox {
	std::mutex rawrbox::RESOURCES::_threadLock;
	std::vector<std::unique_ptr<rawrbox::Loader>> rawrbox::RESOURCES::_loaders = [] {
		std::vector<std::unique_ptr<rawrbox::Loader>> defaults;
		defaults.push_back(std::make_unique<rawrbox::JSONLoader>());
		return defaults;
	}();
} // namespace rawrbox
