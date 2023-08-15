#include <rawrbox/render/light/point.hpp>
#include <rawrbox/render/light/spot.hpp>
#include <rawrbox/render/scripting/wrappers/light/manager_wrapper.hpp>

namespace rawrbox {
	// UTILS ----
	void LightsWrapper::setEnabled(bool fb) {
		rawrbox::LIGHTS::setEnabled(fb);
	}

	// SUN
	void LightsWrapper::setSun(const rawrbox::Vector3f& dir, const rawrbox::Colori& col) {
		rawrbox::LIGHTS::setSun(dir, col.cast<float>());
	}

	const rawrbox::Colori LightsWrapper::getSunColor() {
		return rawrbox::LIGHTS::getSunColor().cast<int>();
	}

	const rawrbox::Vector3f& LightsWrapper::getSunDir() {
		return rawrbox::LIGHTS::getSunDir();
	}
	// ----

	// FOG
	void LightsWrapper::setFog(rawrbox::FOG_TYPE type, float end, float density, const sol::optional<rawrbox::Colori> col) {
		rawrbox::LIGHTS::setFog(type, end, density, col.value_or(rawrbox::Colorsi::White()).cast<float>());
	}

	rawrbox::FOG_TYPE LightsWrapper::getFogType() {
		return rawrbox::LIGHTS::getFogType();
	}

	const rawrbox::Colori LightsWrapper::getFogColor() {
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

	const rawrbox::Colori LightsWrapper::getAmbient() {
		return rawrbox::LIGHTS::getAmbient().cast<int>();
	}
	// -------

	// Light ----

	void LightsWrapper::addPoint(const rawrbox::Vector3f& pos, const rawrbox::Colori& color, float radius) {
		rawrbox::LIGHTS::addLight<rawrbox::PointLight>(pos, color.cast<float>(), radius);
	}

	void LightsWrapper::addSpot(const rawrbox::Vector3f& posMatrix, const rawrbox::Vector3f& direction, const rawrbox::Colori& color, float innerCone, float outerCone, float power) {
		rawrbox::LIGHTS::addLight<rawrbox::SpotLight>(posMatrix, direction, color.cast<float>(), innerCone, outerCone, power);
	}

	void LightsWrapper::removeLight(rawrbox::LightBaseWrapper& light) {
		if (!light.isValid()) return;
		rawrbox::LIGHTS::removeLight(light.getRef());
	}
	// ---------

	// Light utils ----
	rawrbox::LightBaseWrapper LightsWrapper::getLight(size_t indx) {
		auto light = rawrbox::LIGHTS::getLight(indx);
		return {light};
	}

	size_t LightsWrapper::count() {
		return rawrbox::LIGHTS::count();
	}
	// ---------

	void LightsWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<LightsWrapper>("Lights",
		    sol::no_constructor,

		    // UTILS ----
		    "setEnabled", &LightsWrapper::setEnabled,
		    // ---------

		    // SUN
		    "setSun", &LightsWrapper::setFog,
		    "getSunColor", &LightsWrapper::getSunColor,
		    "getSunDir", &LightsWrapper::getSunDir,
		    // ---------

		    // FOG
		    "setFog", &LightsWrapper::setFog,
		    "getFogType", &LightsWrapper::getFogType,
		    "getFogColor", &LightsWrapper::getFogColor,
		    "getFogEnd", &LightsWrapper::getFogEnd,
		    "getFogDensity", &LightsWrapper::getFogDensity,
		    // ---------

		    // AMBIENT
		    "setAmbient", &LightsWrapper::setAmbient,
		    "getAmbient", &LightsWrapper::getAmbient,
		    // ---------

		    // Light
		    "removeLight", &LightsWrapper::removeLight,
		    // ---------

		    // Light utils
		    "getLight", &LightsWrapper::getLight,
		    "count", &LightsWrapper::count
		    // ---------
		);
	}
} // namespace rawrbox
