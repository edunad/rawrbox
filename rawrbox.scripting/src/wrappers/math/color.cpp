
#include <rawrbox/scripting/wrappers/math/color.hpp>

namespace rawrbox {
	void ColorWrapper::registerLua(lua_State* L) {
		registerTemplate<int>(L, "Colori");
		registerTemplate<double>(L, "Colord");
		registerTemplate<float>(L, "Color");
	}
} // namespace rawrbox
