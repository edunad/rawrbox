#include <rawrbox/math/color.hpp>
#include <rawrbox/scripting/wrappers/math/color_wrapper.hpp>

namespace rawrbox {
	void ColorWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::Colori>("Color")
		    .addConstructor<void(), void(rawrbox::Colori), void(int, int, int, int)>()

		    .addProperty("r", &rawrbox::Colori::r)
		    .addProperty("g", &rawrbox::Colori::g)
		    .addProperty("b", &rawrbox::Colori::b)
		    .addProperty("a", &rawrbox::Colori::a)

		    .addFunction("toLinear", &rawrbox::Colori::toLinear)
		    .addFunction("toSRGB", &rawrbox::Colori::toSRGB)
		    .addFunction("toHEX", &rawrbox::Colori::toHEX)
		    .addFunction("lerp", &rawrbox::Colori::lerp)
		    .addFunction("strength", &rawrbox::Colori::strength)

		    .addFunction("rgb", [](const rawrbox::Colori& self) {
			    return self.cast<float>().rgb();
		    })
		    .addFunction("bgr", [](const rawrbox::Colori& self) {
			    return self.cast<float>().bgr();
		    })
		    .addFunction("bgra", [](const rawrbox::Colori& self) {
			    return self.cast<float>().bgra();
		    })

		    .addFunction("invisible", &rawrbox::Colori::invisible)
		    .addFunction("hasTransparency", &rawrbox::Colori::hasTransparency)

		    .addFunction("dot", &rawrbox::Colori::dot)

		    .addFunction("__add", luabridge::overload<int>(&rawrbox::Colori::operator+), luabridge::overload<const Colori&>(&rawrbox::Colori::operator+))
		    .addFunction("__sub", luabridge::overload<int>(&rawrbox::Colori::operator-), luabridge::overload<const Colori&>(&rawrbox::Colori::operator-))
		    .addFunction("__mul", luabridge::overload<int>(&rawrbox::Colori::operator*), luabridge::overload<const Colori&>(&rawrbox::Colori::operator*))
		    .addFunction("__div", luabridge::overload<int>(&rawrbox::Colori::operator/), luabridge::overload<const Colori&>(&rawrbox::Colori::operator/))
		    .addFunction("__eq", &rawrbox::Colori::operator==)
		    .addFunction("__ne", &rawrbox::Colori::operator!=)
		    .addFunction("__unm", [](rawrbox::Colori& color) { return -color; })

		    .addStaticFunction("black", &rawrbox::Colorsi::Black)
		    .addStaticFunction("white", &rawrbox::Colorsi::White)
		    .addStaticFunction("gray", &rawrbox::Colorsi::Gray)
		    .addStaticFunction("red", &rawrbox::Colorsi::Red)
		    .addStaticFunction("green", &rawrbox::Colorsi::Green)
		    .addStaticFunction("blue", &rawrbox::Colorsi::Blue)
		    .addStaticFunction("orange", &rawrbox::Colorsi::Orange)
		    .addStaticFunction("yellow", &rawrbox::Colorsi::Yellow)
		    .addStaticFunction("purple", &rawrbox::Colorsi::Purple)
		    .addStaticFunction("brown", &rawrbox::Colorsi::Brown)
		    .addStaticFunction("transparent", &rawrbox::Colorsi::Transparent)

		    .endClass();
	}
} // namespace rawrbox
