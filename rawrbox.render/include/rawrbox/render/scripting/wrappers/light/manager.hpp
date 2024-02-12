#pragma once

#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class LightsWrapper {
	public:
		// UTILS ----
		static void setEnabled(bool fb);

		// FOG
		static void setFog(rawrbox::FOG_TYPE type, float end, float density, const std::optional<rawrbox::Colori> col);

		static int getFogType();
		static rawrbox::Colori getFogColor();
		static float getFogEnd();
		static float getFogDensity();
		// ----

		// AMBIENT
		static void setAmbient(const rawrbox::Colori& col);
		static rawrbox::Colori getAmbient();
		// -------

		// Light ----
		static void addPoint(const rawrbox::Vector3f& pos, const rawrbox::Colori& color, float radius);
		static void addSpot(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colori& color, float innerCone, float outerCone, float power);
		static void addDirectional(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colori& color);

		static void removeLight(rawrbox::LightBase& light);
		// ---------

		// utils ----
		static rawrbox::LightBase* get(int indx);
		static size_t count();
		// ---------

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
