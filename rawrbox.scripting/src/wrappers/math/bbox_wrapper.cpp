#include <rawrbox/math/bbox.hpp>
#include <rawrbox/scripting/wrappers/math/bbox_wrapper.hpp>

namespace rawrbox {
	void BBOXWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::BBOX>("BBOX")

		    .addConstructor<void(), void(rawrbox::BBOX), void(const rawrbox::Vector3&, const rawrbox::Vector3&, const rawrbox::Vector3&)>()

		    .addProperty("min", &rawrbox::BBOX::_min)
		    .addProperty("max", &rawrbox::BBOX::_max)
		    .addProperty("size", &rawrbox::BBOX::_size)

		    .addFunction("isEmpty", &rawrbox::BBOX::isEmpty)
		    .addFunction("size", &rawrbox::BBOX::size)
		    .addFunction("combine", &rawrbox::BBOX::combine)

		    .addFunction("__eq", &rawrbox::BBOX::operator==)
		    .addFunction("__ne", &rawrbox::BBOX::operator!=)

		    .endClass();
	}
} // namespace rawrbox
