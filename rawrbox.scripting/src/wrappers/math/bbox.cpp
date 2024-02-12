
#include <rawrbox/scripting/wrappers/math/bbox.hpp>

namespace rawrbox {
	void BBOXWrapper::registerLua(lua_State* L) {
		registerTemplate<int>(L, "BBOXi");
		registerTemplate<double>(L, "BBOXd");
		registerTemplate<float>(L, "BBOX");
	}
} // namespace rawrbox
