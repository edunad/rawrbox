
#include <rawrbox/scripting/wrappers/math/vector3.hpp>

namespace rawrbox {
	void Vector3Wrapper::registerLua(lua_State* L) {
		Vector3Wrapper::registerTemplate<int>(L, "Vector3i");
		Vector3Wrapper::registerTemplate<double>(L, "Vector3d");
		Vector3Wrapper::registerTemplate<float>(L, "Vector3");
	}
} // namespace rawrbox
