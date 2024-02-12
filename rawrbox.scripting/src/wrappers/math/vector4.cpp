
#include <rawrbox/scripting/wrappers/math/vector4.hpp>

namespace rawrbox {
	void Vector4Wrapper::registerLua(lua_State* L) {
		registerTemplate<int>(L, "Vector4i");
		registerTemplate<double>(L, "Vector4d");
		registerTemplate<float>(L, "Vector4");
	}
} // namespace rawrbox
