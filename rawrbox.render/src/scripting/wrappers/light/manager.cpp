#include <rawrbox/render/lights/directional.hpp>
#include <rawrbox/render/lights/point.hpp>
#include <rawrbox/render/lights/spot.hpp>
#include <rawrbox/render/scripting/wrappers/light/manager.hpp>

namespace rawrbox {
	// UTILS ----
	void LightsWrapper::setEnabled(bool fb) {
		rawrbox::LIGHTS::setEnabled(fb);
	}

	// FOG
	void LightsWrapper::setFog(rawrbox::FOG_TYPE type, float end, float density, const std::optional<rawrbox::Colori> col) {
		rawrbox::LIGHTS::setFog(type, end, density, col.value_or(rawrbox::Colorsi::White()).cast<float>());
	}

	int LightsWrapper::getFogType() {
		return static_cast<int>(rawrbox::LIGHTS::getFogType());
	}

	rawrbox::Colori LightsWrapper::getFogColor() {
		return rawrbox::LIGHTS::getFogColor().cast<int>();
	}

	float LightsWrapper::getFogEnd() {
		return rawrbox::LIGHTS::getFogEnd();
	}

	float LightsWrapper::getFogDensity() {
		return rawrbox::LIGHTS::getFogDensity();
	}
	// ----

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
		rawrbox::LIGHTS::addLight<rawrbox::PointLight>(pos, color.cast<float>(), radius);
	}

	void LightsWrapper::addSpot(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colori& color, float innerCone, float outerCone, float power) {
		rawrbox::LIGHTS::addLight<rawrbox::SpotLight>(pos, direction, color.cast<float>(), innerCone, outerCone, power);
	}

	void LightsWrapper::addDirectional(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colori& color) {
		rawrbox::LIGHTS::addLight<rawrbox::DirectionalLight>(pos, direction, color.cast<float>());
	}

	void LightsWrapper::removeLight(rawrbox::LightBase& light) {
		rawrbox::LIGHTS::removeLight(light.id());
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
		    .beginNamespace("lights")
		    .addFunction("setEnabled", &LightsWrapper::setEnabled)

		    // FOG ---
		    .addFunction("setFog", &LightsWrapper::setFog)
		    .addFunction("getFogType", &LightsWrapper::getFogType)
		    .addFunction("getFogColor", &LightsWrapper::getFogColor)
		    .addFunction("getFogEnd", &LightsWrapper::getFogEnd)
		    .addFunction("getFogDensity", &LightsWrapper::getFogDensity)
		    // --------

		    // AMBIENT ---
		    .addFunction("setAmbient", &LightsWrapper::setAmbient)
		    .addFunction("getAmbient", &LightsWrapper::getAmbient)
		    // --------

		    // LIGHT ---
		    .addFunction("addPoint", &LightsWrapper::addPoint)
		    .addFunction("addSpot", &LightsWrapper::addSpot)
		    .addFunction("addDirectional", &LightsWrapper::addDirectional)
		    //
		    .addFunction("removeLight", &LightsWrapper::removeLight)
		    // --------

		    // UTIL ---
		    .addFunction("get", &LightsWrapper::get)
		    .addFunction("count", &LightsWrapper::count)
		    // --------

		    .endNamespace();
	}
} // namespace rawrbox
