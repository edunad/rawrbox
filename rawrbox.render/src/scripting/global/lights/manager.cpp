#include <rawrbox/render/lights/directional.hpp>
#include <rawrbox/render/lights/point.hpp>
#include <rawrbox/render/lights/spot.hpp>
#include <rawrbox/render/scripting/global/lights/manager.hpp>

namespace rawrbox {
	// UTILS ----
	void LightsGlobal::setEnabled(bool fb) {
		rawrbox::LIGHTS::setEnabled(fb);
	}

	// AMBIENT
	void LightsGlobal::setAmbient(const rawrbox::Colori& col) {
		rawrbox::LIGHTS::setAmbient(col.cast<float>());
	}

	rawrbox::Colori LightsGlobal::getAmbient() {
		return rawrbox::LIGHTS::getAmbient().cast<int>();
	}
	// -------

	// Light ----
	void LightsGlobal::addPoint(const rawrbox::Vector3f& pos, const rawrbox::Colori& color, float radius) {
		rawrbox::LIGHTS::add<rawrbox::PointLight>(pos, color.cast<float>(), radius);
	}

	void LightsGlobal::addSpot(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colori& color, float innerCone, float outerCone, float power) {
		rawrbox::LIGHTS::add<rawrbox::SpotLight>(pos, direction, color.cast<float>(), innerCone, outerCone, power);
	}

	void LightsGlobal::addDirectional(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colori& color) {
		rawrbox::LIGHTS::add<rawrbox::DirectionalLight>(pos, direction, color.cast<float>());
	}

	void LightsGlobal::remove(const rawrbox::LightBase& light) {
		rawrbox::LIGHTS::remove(light);
	}

	void LightsGlobal::remove(size_t index) {
		rawrbox::LIGHTS::remove(index);
	}

	void LightsGlobal::clear() {
		rawrbox::LIGHTS::clear();
	}
	// ---------

	// Light utils ----
	rawrbox::LightBase* LightsGlobal::get(int indx) {
		if (indx < 0) return nullptr;
		return rawrbox::LIGHTS::getLight(indx - 1); // Lua starts at 1
	}

	size_t LightsGlobal::count() {
		return rawrbox::LIGHTS::count();
	}
	// ---------

	void LightsGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("lights", {})
		    .addFunction("setEnabled", &LightsGlobal::setEnabled)

		    // AMBIENT ---
		    .addFunction("setAmbient", &LightsGlobal::setAmbient)
		    .addFunction("getAmbient", &LightsGlobal::getAmbient)
		    // --------

		    // LIGHT ---
		    .addFunction("addPoint", &LightsGlobal::addPoint)
		    .addFunction("addSpot", &LightsGlobal::addSpot)
		    .addFunction("addDirectional", &LightsGlobal::addDirectional)
		    //
		    .addFunction("remove",
			luabridge::overload<const rawrbox::LightBase&>(&LightsGlobal::remove),
			luabridge::overload<size_t>(&LightsGlobal::remove))
		    .addFunction("clear", &LightsGlobal::clear)
		    // --------

		    // UTIL ---
		    .addFunction("get", &LightsGlobal::get)
		    .addFunction("count", &LightsGlobal::count)
		    // --------

		    .endNamespace();
	}
} // namespace rawrbox
