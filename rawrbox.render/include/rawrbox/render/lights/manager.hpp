#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/lights/base.hpp>
#include <rawrbox/render/static.hpp>

#include <DynamicBuffer.hpp>

#include <memory>
#include <vector>

namespace rawrbox {

	enum class FOG_TYPE : uint32_t {
		FOG_LINEAR = 0,
		FOG_EXP = 1
	};

	struct LightDataVertex {
		rawrbox::Vector4f position = {};
		rawrbox::Vector4f direction = {};
		rawrbox::Vector3f color = {};
		float intensity = 1.F;

		float radius = 0.F;
		float penumbra = 0.F;
		float umbra = 0.F;

		rawrbox::LightType type = rawrbox::LightType::UNKNOWN;
	};

	struct LightConstants {
		// Light ---------
		rawrbox::Vector4_t<uint32_t> lightSettings = {};
		// ------

		// Ambient ---
		rawrbox::Colorf ambientColor = {0.01F, 0.01F, 0.01F, 1.F};
		// -----

		// Fog ----
		rawrbox::Colorf fogColor = {0.F, 0.F, 0.F, 0.F};
		rawrbox::Vector4f fogSettings = {static_cast<float>(rawrbox::FOG_TYPE::FOG_EXP), -1.F, -1.F, 0};
		// --------
	};

	class LIGHTS {
	protected:
		static std::vector<std::shared_ptr<rawrbox::LightBase>> _lights;
		static rawrbox::LightConstants _settings;

		static std::unique_ptr<Diligent::DynamicBuffer> _buffer;
		static Diligent::IBufferView* _bufferRead;

		static bool _CONSTANTS_DIRTY;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static void updateConstants();
		static void update();

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		static void init();
		static void shutdown();

		static void bindUniforms();

		// UTILS ----
		static void setEnabled(bool fb);

		static rawrbox::LightBase* getLight(size_t indx);
		static size_t count();

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
			_CONSTANTS_DIRTY = true;
			return light;
		}

		static bool removeLight(size_t indx);
		static bool removeLight(rawrbox::LightBase* light);
		// ---------
	};
} // namespace rawrbox
