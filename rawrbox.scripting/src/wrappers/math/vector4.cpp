
#include <rawrbox/scripting/wrappers/math/vector4.hpp>

namespace rawrbox {
	void Vector4Wrapper::registerLua(lua_State* L) {
		Vector4Wrapper::registerTemplate<int>(L, "Vector4i");
		Vector4Wrapper::registerTemplate<double>(L, "Vector4d");
		Vector4Wrapper::registerTemplate<float>(L, "Vector4");
	}
} // namespace rawrbox
