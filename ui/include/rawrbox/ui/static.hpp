#pragma once
#include <rawrbox/ui/root.hpp>

#include <string>

namespace rawrbox {
	extern std::shared_ptr<rawrbox::UIRoot> ROOT_UI;
	extern std::vector<std::pair<std::string, uint32_t>> UI_RESOURCES;
} // namespace rawrbox
