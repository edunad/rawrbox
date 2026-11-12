
#include <rawrbox/scripting/wrappers/math/aabb.hpp>

namespace rawrbox {
	void AABBWrapper::registerLua(lua_State* L) {
		AABBWrapper::registerTemplate<int>(L, "AABBi");
		AABBWrapper::registerTemplate<double>(L, "AABBd");
		AABBWrapper::registerTemplate<float>(L, "AABB");
	}
} // namespace rawrbox
