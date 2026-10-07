#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/lights/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/clustered.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

#include <array>

namespace rawrbox {
	class CameraBase;
	struct LightDataVertex {
		rawrbox::Vector3f position = {};
		float radius = 0.F;

		rawrbox::Vector3f direction = {};
		uint32_t type = 0;

		// SPOT LIGHT ONLY
		float cosUmbra = 0.F;
		float cosPenumbra = 0.F;
		// ---------------

		std::array<uint32_t, 2> radiance = {};

		static constexpr uint32_t TYPE_MASK = 0x3U;
		static constexpr uint32_t SHADOW_SHIFT = 2U; // TODO: shadow
	};

	struct LightConstants {
		// Light (x = enabled, y = total, z = directional, w = debug) ---------
		rawrbox::Vector4u lightSettings = {1U, 0U, 0U, 0U};
		// ------

		// Ambient ---
		rawrbox::Colorf ambientColor = {0.1F, 0.1F, 0.1F, 1.F};
		// -----
	};

	class LIGHTS {
	protected:
		static std::vector<std::shared_ptr<rawrbox::LightBase>> _lights;
		static rawrbox::LightConstants _settings;

		// BINNING ---
		static std::vector<rawrbox::LightDataVertex> _data;
		static std::vector<rawrbox::LightDataVertex> _sorted; // Directional -> then sorted by view depth

		static std::vector<rawrbox::Vector4f> _viewBounds;
		static std::vector<rawrbox::Vector4f> _bounds;

		static std::vector<rawrbox::ZBinEntry> _entries;
		static std::vector<uint32_t> _zbins;

		static rawrbox::Matrix4x4 _zbinView;
		static rawrbox::Vector2f _zbinNearFar;
		// -----------

		// BUFFERS ---
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _buffer;
		static Diligent::IBufferView* _bufferRead;

		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _zbinBuffer;
		static Diligent::IBufferView* _zbinBufferRead;

		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _boundsBuffer;
		static Diligent::IBufferView* _boundsBufferRead;
		// -----------

		static bool _BINS_DIRTY;
		static bool _CONSTANTS_DIRTY;
		static bool _BINNED;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static void updateData();
		static bool updateBins(const rawrbox::CameraBase& camera);

		static void updateConstants();
		static rawrbox::LightConstants getGPUConstants();

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		static void init();
		static void shutdown();

		static bool update(const rawrbox::CameraBase& camera, bool binned = true);

		// UTILS ----
		static void setEnabled(bool enabled);
		static bool isEnabled();

		static void setDebug(bool enabled);
		static bool isDebug();

		static rawrbox::LightBase* getLight(size_t indx);

		static size_t count();
		static size_t active();

		static Diligent::IBufferView* getBuffer();
		static Diligent::IBufferView* getZBinBuffer();
		static Diligent::IBufferView* getBoundsBuffer();

		static rawrbox::Vector4f getBounds(const rawrbox::LightDataVertex& light);
		// ----------

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
