#include <rawrbox/math/color.hpp>
#include <rawrbox/scripting/wrappers/math/color_wrapper.hpp>

namespace rawrbox {
	void ColorWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::Colori>("Color",
		    sol::constructors<Colori(), Colori(Colori), Colori(int, int, int, int)>(),

		    "r", &Colori::r,
		    "g", &Colori::g,
		    "b", &Colori::b,
		    "a", &Colori::a,

		    "pack", &Colori::pack,
		    "dot", &Colori::dot,
		    "max", &Colori::max,
		    "data", &Colori::data,
		    "lerp", &Colori::lerp,
		    "debug", &Colori::debug,
		    "hasTransparency", &Colori::hasTransparency,
		    "isTransparent", &Colori::isTransparent,

		    sol::meta_function::addition, sol::resolve<Colori(const Colori&) const>(&Colori::operator+),
		    sol::meta_function::addition, sol::resolve<Colori(int) const>(&Colori::operator+),

		    sol::meta_function::subtraction, sol::resolve<Colori(const Colori&) const>(&Colori::operator-),
		    sol::meta_function::subtraction, sol::resolve<Colori(int) const>(&Colori::operator-),

		    sol::meta_function::division, sol::resolve<Colori(const Colori&) const>(&Colori::operator/),
		    sol::meta_function::division, sol::resolve<Colori(int) const>(&Colori::operator/),

		    sol::meta_function::multiplication, sol::resolve<Colori(const Colori&) const>(&Colori::operator*),
		    sol::meta_function::multiplication, sol::resolve<Colori(int) const>(&Colori::operator*));
	}
} // namespace rawrbox
