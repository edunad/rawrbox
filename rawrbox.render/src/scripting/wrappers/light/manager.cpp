#include <rawrbox/render/lights/directional.hpp>
#include <rawrbox/render/lights/point.hpp>
#include <rawrbox/render/lights/spot.hpp>
#include <rawrbox/render/scripting/wrappers/light/manager.hpp>

namespace rawrbox {
	// UTILS ----
	void LightsWrapper::setEnabled(bool fb) {
		rawrbox::LIGHTS::setEnabled(fb);
	}

	// AMBIENT
	void LightsWrapper::setAmbient(const rawrbox::Colori& col) {
		rawrbox::LIGHTS::setAmbient(col.cast<float>());
	}

	rawrbox::Colori LightsWrapper::getAmbient() {
		return rawrbox::LIGHTS::getAmbient().cast<int>();
	}
	// -------

	// Light ----
	void LightsWrapper::addPoint(const rawrbox::Vector3f& pos, const rawrbox::Colori& color, float radius) {
		rawrbox::LIGHTS::add<rawrbox::PointLight>(pos, color.cast<float>(), radius);
	}

	void LightsWrapper::addSpot(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colori& color, float innerCone, float outerCone, float power) {
		rawrbox::LIGHTS::add<rawrbox::SpotLight>(pos, direction, color.cast<float>(), innerCone, outerCone, power);
	}

	void LightsWrapper::addDirectional(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colori& color) {
		rawrbox::LIGHTS::add<rawrbox::DirectionalLight>(pos, direction, color.cast<float>());
	}

	void LightsWrapper::remove(const rawrbox::LightBase& light) {
		rawrbox::LIGHTS::remove(light);
	}

	void LightsWrapper::remove(size_t index) {
		rawrbox::LIGHTS::remove(index);
	}

	void LightsWrapper::clear() {
		rawrbox::LIGHTS::clear();
	}
	// ---------

	// Light utils ----
	rawrbox::LightBase* LightsWrapper::get(int indx) {
		if (indx < 0) return nullptr;
		return rawrbox::LIGHTS::getLight(indx - 1); // Lua starts at 1
	}

	size_t LightsWrapper::count() {
		return rawrbox::LIGHTS::count();
	}
	// ---------

	void LightsWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("lights", {})
		    .addFunction("setEnabled", &LightsWrapper::setEnabled)

		    // AMBIENT ---
		    .addFunction("setAmbient", &LightsWrapper::setAmbient)
		    .addFunction("getAmbient", &LightsWrapper::getAmbient)
		    // --------

		    // LIGHT ---
		    .addFunction("addPoint", &LightsWrapper::addPoint)
		    .addFunction("addSpot", &LightsWrapper::addSpot)
		    .addFunction("addDirectional", &LightsWrapper::addDirectional)
		    //
		    .addFunction("remove",
			luabridge::overload<const rawrbox::LightBase&>(&LightsWrapper::remove),
			luabridge::overload<size_t>(&LightsWrapper::remove))
		    .addFunction("clear", &LightsWrapper::clear)
		    // --------

		    // UTIL ---
		    .addFunction("get", &LightsWrapper::get)
		    .addFunction("count", &LightsWrapper::count)
		    // --------

		    .endNamespace();
	}
} // namespace rawrbox
