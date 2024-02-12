
#include <rawrbox/scripting/wrappers/math/aabb.hpp>

namespace rawrbox {
	void AABBWrapper::registerLua(lua_State* L) {
		registerTemplate<int>(L, "AABBi");
		registerTemplate<double>(L, "AABBd");
		registerTemplate<float>(L, "AABB");
	}
} // namespace rawrbox
