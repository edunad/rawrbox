#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/lights/base.hpp>
#include <rawrbox/render/static.hpp>

#include <DynamicBuffer.hpp>

namespace rawrbox {

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
		rawrbox::Vector4u lightSettings = {1U, 0U, 0U, 0U};
		// ------

		// Ambient ---
		rawrbox::Colorf ambientColor = {0.01F, 0.01F, 0.01F, 1.F};
		// -----
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

		static void createDataBuffer();

		static void updateConstants();
		static void update();

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;
		static std::function<void()> onUpdate;

		static void init();
		static void shutdown();

		static void bindUniforms();

		// UTILS ----
		static void setEnabled(bool enabled);
		static bool isEnabled();

		static rawrbox::LightBase* getLight(size_t indx);
		static size_t count();

		static Diligent::IBufferView* getBuffer();

		// AMBIENT
		static void setAmbient(const rawrbox::Colorf& col);
		static const rawrbox::Colorf& getAmbient();
		// -------

		// Light ----
		template <typename T = rawrbox::LightBase, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::LightBase>)
		static rawrbox::LightBase* add(CallbackArgs&&... args) {
			auto light = _lights.emplace_back(std::make_shared<T>(std::forward<CallbackArgs>(args)...)).get();
			light->setId(++rawrbox::LIGHT_ID);

			rawrbox::__LIGHT_DIRTY__ = true;
			_CONSTANTS_DIRTY = true;
			return light;
		}

		static bool remove(size_t indx);
		static bool remove(const rawrbox::LightBase& light);
		static void clear();
		// ---------
	};
} // namespace rawrbox
