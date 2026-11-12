
#include <rawrbox/scripting/wrappers/math/color.hpp>

namespace rawrbox {
	void ColorWrapper::registerLua(lua_State* L) {
		ColorWrapper::registerTemplate<int>(L, "Colori");
		ColorWrapper::registerTemplate<double>(L, "Colord");
		ColorWrapper::registerTemplate<float>(L, "Color");
	}
} // namespace rawrbox