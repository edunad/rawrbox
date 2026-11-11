
#include <rawrbox/scripting/wrappers/math/bbox.hpp>

namespace rawrbox {
	void BBOXWrapper::registerLua(lua_State* L) {
		BBOXWrapper::registerTemplate<int>(L, "BBOXi");
		BBOXWrapper::registerTemplate<double>(L, "BBOXd");
		BBOXWrapper::registerTemplate<float>(L, "BBOX");
	}
} // namespace rawrbox
