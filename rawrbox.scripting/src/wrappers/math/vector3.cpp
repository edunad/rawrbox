
#include <rawrbox/scripting/wrappers/math/vector3.hpp>

namespace rawrbox {
	void Vector3Wrapper::registerLua(lua_State* L) {
		registerTemplate<int>(L, "Vector3i");
		registerTemplate<double>(L, "Vector3d");
		registerTemplate<float>(L, "Vector3");
	}
} // namespace rawrbox
