#include <rawrbox/resources/manager.hpp>

namespace rawrbox {
	std::mutex rawrbox::RESOURCES::_threadLock;
	std::vector<std::unique_ptr<rawrbox::Loader>> rawrbox::RESOURCES::_loaders = {};
} // namespace rawrbox
