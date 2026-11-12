
#include <rawrbox/scripting/wrappers/math/vector2.hpp>

namespace rawrbox {
	void Vector2Wrapper::registerLua(lua_State* L) {
		Vector2Wrapper::registerTemplate<int>(L, "Vector2i");
		Vector2Wrapper::registerTemplate<double>(L, "Vector2d");
		Vector2Wrapper::registerTemplate<float>(L, "Vector2");
	}
} // namespace rawrbox
