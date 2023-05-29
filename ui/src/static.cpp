#include <rawrbox/ui/static.hpp>

namespace rawrbox {
	std::vector<std::pair<std::string, uint32_t>> UI_RESOURCES = {
	    // FONTS
	    std::make_pair<std::string, uint32_t>("consola.ttf", 0), // TODO: ADD LINUX FONT
	    // ICONS
	    std::make_pair<std::string, uint32_t>("./content/textures/ui/icons/close.png", 0),
	    // OVERLAYS
	    std::make_pair<std::string, uint32_t>("./content/textures/ui/overlay/overlay.png", 0),
	    // OTHER
	    std::make_pair<std::string, uint32_t>("./content/textures/ui/background_grid.png", 0),
	    std::make_pair<std::string, uint32_t>("./content/textures/ui/border-line.png", 0),
	    std::make_pair<std::string, uint32_t>("./content/textures/ui/stripe.png", 0)};
} // namespace rawrbox
