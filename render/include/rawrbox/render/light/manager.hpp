#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/light/base.hpp>
#include <rawrbox/render/static.hpp>

#include <memory>
#include <vector>

namespace rawrbox {

	struct LightDataVertex {
		rawrbox::Vector3f position = {};
		float outerCone = 0.F;

		rawrbox::Vector3f intensity = {};
		float radius = 0.F;

		rawrbox::Vector3f direction = {};
		float innerCone = 0.F;

		uint32_t type = 0;

		static std::array<Diligent::LayoutElement, 7> vLayout() {
			return {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - Outer Cone
			    Diligent::LayoutElement{1, 0, 1, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Intensity
			    Diligent::LayoutElement{2, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 3 - Radius
			    Diligent::LayoutElement{3, 0, 1, Diligent::VT_FLOAT32, false},
			    // Attribute 4 - Direction
			    Diligent::LayoutElement{4, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 5 - InnerCone
			    Diligent::LayoutElement{5, 0, 1, Diligent::VT_FLOAT32, false},
			    // Attribute 6 - Type
			    Diligent::LayoutElement{6, 0, 1, Diligent::VT_UINT32, false},
			};
		}
	};

	enum class FOG_TYPE {
		FOG_LINEAR = 0,
		FOG_EXP = 1
	};

	struct LightConstants {
		// Light ---------
		std::array<uint32_t, 4> g_LightSettings = {};
		// ------

		// Sun ----
		rawrbox::Vector3f g_SunDirection = {};
		rawrbox::Vector3f g_SunColor = {};
		// ----

		// Ambient ---
		rawrbox::Vector3f g_AmbientColor = {};
		// -----

		// Fog ----
		rawrbox::Vector3f g_FogColor = {};
		rawrbox::Vector4f g_FogSettings = {};
		// --------
	};

	class LIGHTS {
	protected:
		static std::vector<std::shared_ptr<rawrbox::LightBase>> _lights;

		static rawrbox::Colorf _ambient;

		static rawrbox::Colorf _sun_color;
		static rawrbox::Vector3f _sun_direction;

		static rawrbox::Colorf _fog_color;
		static rawrbox::FOG_TYPE _fog_type;
		static float _fog_density;
		static float _fog_end;

		static void update();

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;

		static bool fullbright;

		static void init();
		static void shutdown();

		static void bindUniforms();

		// UTILS ----
		static void setEnabled(bool fb);

		// SUN
		static void setSun(const rawrbox::Vector3f& dir, const rawrbox::Colorf& col);
		static const rawrbox::Colorf& getSunColor();
		static const rawrbox::Vector3f& getSunDir();
		// ----

		// FOG
		static void setFog(rawrbox::FOG_TYPE type, float end, float density, const rawrbox::Colorf& col = rawrbox::Colors::Black());

		static rawrbox::FOG_TYPE getFogType();
		static const rawrbox::Colorf& getFogColor();
		static float getFogEnd();
		static float getFogDensity();
		// ----

		// AMBIENT
		static void setAmbient(const rawrbox::Colorf& col);
		static const rawrbox::Colorf& getAmbient();
		// -------

		// Light ----
		template <typename T = rawrbox::LightBase, typename... CallbackArgs>
		static rawrbox::LightBase* addLight(CallbackArgs&&... args) {
			auto light = _lights.emplace_back(std::make_shared<T>(std::forward<CallbackArgs>(args)...)).get();
			light->setId(++rawrbox::LIGHT_ID);
			rawrbox::__LIGHT_DIRTY__ = true;

			return light;
		}

		static void removeLight(rawrbox::LightBase* light);
		// ---------

		// Light utils ----
		static rawrbox::LightBase* getLight(size_t indx);
		static size_t count();
		// ---------
	};
} // namespace rawrbox
