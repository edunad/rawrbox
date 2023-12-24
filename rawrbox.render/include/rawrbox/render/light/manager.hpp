#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/light/base.hpp>
#include <rawrbox/render/static.hpp>

#include <DynamicBuffer.hpp>

#include <memory>
#include <vector>

namespace rawrbox {

	struct LightDataVertex {
		rawrbox::Vector4f position = {};
		rawrbox::Vector4f direction = {};

		rawrbox::Vector3f color = {};
		float intensity = 10.F;

		float radius = 0.F;
		float penumbra = 0.F;
		float umbra = 0.F;

		rawrbox::LightType type = rawrbox::LightType::UNKNOWN;
	};

	enum class FOG_TYPE {
		FOG_LINEAR = 0,
		FOG_EXP = 1
	};

	struct LightConstants {
		// Light ---------
		std::array<uint32_t, 4> g_LightSettings = {};
		// ------

		// Ambient ---
		rawrbox::Colorf g_AmbientColor = {};
		// -----

		// Fog ----
		rawrbox::Colorf g_FogColor = {};
		rawrbox::Vector4f g_FogSettings = {};
		// --------

		rawrbox::Vector4f g_LightGridParams = {};
	};

	class LIGHTS {
	protected:
		static std::vector<std::shared_ptr<rawrbox::LightBase>> _lights;

		static rawrbox::Colorf _ambient;
		static rawrbox::Colorf _fog_color;
		static rawrbox::FOG_TYPE _fog_type;
		static float _fog_density;
		static float _fog_end;

		static std::unique_ptr<Diligent::DynamicBuffer> _buffer;
		static Diligent::IBufferView* _bufferRead;

		static void update();

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		static bool fullbright;

		static void init();
		static void shutdown();

		static void bindUniforms();

		// UTILS ----
		static void setEnabled(bool fb);
		static Diligent::IBufferView* getBuffer();

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

		static bool removeLight(size_t indx);
		static bool removeLight(rawrbox::LightBase* light);
		// ---------

		// Light utils ----
		static rawrbox::LightBase* getLight(size_t indx);
		static size_t count();
		// ---------
	};
} // namespace rawrbox
