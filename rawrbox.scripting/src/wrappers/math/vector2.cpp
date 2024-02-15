
#include <rawrbox/scripting/wrappers/math/vector2.hpp>

namespace rawrbox {
	void Vector2Wrapper::registerLua(lua_State* L) {
		registerTemplate<int>(L, "Vector2i");
		registerTemplate<double>(L, "Vector2d");
		registerTemplate<float>(L, "Vector2");
	}
} // namespace rawrbox
