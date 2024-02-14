#pragma once
#include <rawrbox/math/color.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class ColorWrapper {
		template <typename T = int>
			requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
		static void registerTemplate(lua_State* L, const std::string& name) {
			using CL = rawrbox::Color_t<T>;
			using CLS = rawrbox::Colors_t<T>;

			luabridge::getGlobalNamespace(L)
			    .beginClass<CL>(name.c_str())
			    .template addConstructor<void(), void(CL), void(T, T, T, T)>()
			    .addProperty("r", &CL::r)
			    .addProperty("g", &CL::g)
			    .addProperty("b", &CL::b)
			    .addProperty("a", &CL::a)

			    .addFunction("toLinear", &CL::toLinear)
			    .addFunction("toSRGB", &CL::toSRGB)
			    .addFunction("toHEX", &CL::toHEX)
			    .addFunction("lerp", &CL::lerp)
			    .addFunction("strength", &CL::strength)
			    .addFunction("invisible", &CL::invisible)
			    .addFunction("hasTransparency", &CL::hasTransparency)
			    .addFunction("dot", &CL::dot)

			    .addFunction("rgb", [](const CL& self) {
				    return self.template cast<float>().rgb();
			    })
			    .addFunction("bgr", [](const CL& self) {
				    return self.template cast<float>().bgr();
			    })
			    .addFunction("bgra", [](const CL& self) {
				    return self.template cast<float>().bgra();
			    })

			    .addFunction("__add", luabridge::overload<T>(&CL::operator+), luabridge::overload<const CL&>(&CL::operator+))
			    .addFunction("__sub", luabridge::overload<T>(&CL::operator-), luabridge::overload<const CL&>(&CL::operator-))
			    .addFunction("__mul", luabridge::overload<T>(&CL::operator*), luabridge::overload<const CL&>(&CL::operator*))
			    .addFunction("__div", luabridge::overload<T>(&CL::operator/), luabridge::overload<const CL&>(&CL::operator/))
			    .addFunction("__eq", &CL::operator==)
			    .addFunction("__ne", &CL::operator!=)
			    .addFunction("__unm", [](CL& color) { return -color; })

			    .addStaticFunction("black", &CLS::Black)
			    .addStaticFunction("white", &CLS::White)
			    .addStaticFunction("gray", &CLS::Gray)
			    .addStaticFunction("red", &CLS::Red)
			    .addStaticFunction("green", &CLS::Green)
			    .addStaticFunction("blue", &CLS::Blue)
			    .addStaticFunction("orange", &CLS::Orange)
			    .addStaticFunction("yellow", &CLS::Yellow)
			    .addStaticFunction("purple", &CLS::Purple)
			    .addStaticFunction("brown", &CLS::Brown)
			    .addStaticFunction("transparent", &CLS::Transparent)

			    .endClass();
		}

	public:
		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
