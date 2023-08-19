#pragma once

#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/scripting/wrappers/light/base_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class LightsWrapper {
	public:
		LightsWrapper() = default;
		LightsWrapper(const LightsWrapper&) = default;
		LightsWrapper(LightsWrapper&&) = default;
		LightsWrapper& operator=(const LightsWrapper&) = default;
		LightsWrapper& operator=(LightsWrapper&&) = default;
		virtual ~LightsWrapper() = default;

		// UTILS ----
		virtual void setEnabled(bool fb);

		// SUN
		virtual void setSun(const rawrbox::Vector3f& dir, const rawrbox::Colori& col);
		virtual const rawrbox::Colori getSunColor();
		virtual const rawrbox::Vector3f& getSunDir();
		// ----

		// FOG
		virtual void setFog(rawrbox::FOG_TYPE type, float end, float density, const sol::optional<rawrbox::Colori> col);

		virtual rawrbox::FOG_TYPE getFogType();
		virtual const rawrbox::Colori getFogColor();
		virtual float getFogEnd();
		virtual float getFogDensity();
		// ----

		// AMBIENT
		virtual void setAmbient(const rawrbox::Colori& col);
		virtual const rawrbox::Colori getAmbient();
		// -------

		// Light ----
		void addPoint(const rawrbox::Vector3f& pos, const rawrbox::Colori& color, float radius);
		void addSpot(const rawrbox::Vector3f& posMatrix, const rawrbox::Vector3f& direction, const rawrbox::Colori& color, float innerCone, float outerCone, float power);

		void removeLight(rawrbox::LightBaseWrapper& light);
		// ---------

		// Light utils ----
		virtual sol::object getLight(size_t indx);
		virtual size_t count();
		// ---------

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
