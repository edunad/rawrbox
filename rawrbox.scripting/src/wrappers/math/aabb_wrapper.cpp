#include <rawrbox/math/aabb.hpp>
#include <rawrbox/scripting/wrappers/math/aabb_wrapper.hpp>

namespace rawrbox {
	void AABBWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::AABB>("AABB")
		    .addConstructor<void(), void(rawrbox::AABB), void(float, float, float, float), void(const Vector2_t<float>&, const Vector2_t<float>&)>()

		    .addProperty("position", &AABB::pos)
		    .addProperty("size", &AABB::size)

		    .addFunction("surfaceArea", &AABB::surfaceArea)
		    .addFunction("top", &AABB::top)
		    .addFunction("right", &AABB::right)
		    .addFunction("left", &AABB::left)
		    .addFunction("bottom", &AABB::bottom)
		    .addFunction("empty", &AABB::empty)

		    .addFunction("contains",
			luabridge::overload<const Vector2&>(&AABB::contains),
			luabridge::overload<const AABB&>(&AABB::contains))

		    .addFunction("intersects", &AABB::intersects)
		    .addFunction("mask", &AABB::mask)

		    .addFunction("__mul", &AABB::operator*)
		    .addFunction("__eq", &AABB::operator==)
		    .addFunction("__ne", &AABB::operator!=)

		    .endClass();
	}
} // namespace rawrbox
