#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/decals/decal.hpp>
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/utils/clustered.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

namespace rawrbox {
	class CameraBase;

	class DECALS {
	protected:
		static std::vector<rawrbox::Decal> _decals;

		// BINNING ---
		static std::vector<rawrbox::Decal> _sorted; // By depth

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

		static bool _CONSTANTS_DIRTY;
		static bool _BINNED;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static void updateConstants();
		static bool updateBins(const rawrbox::CameraBase& camera);

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		static void init();
		static void shutdown();

		static bool update(const rawrbox::CameraBase& camera, bool binned = true);

		// UTILS ----
		static Diligent::IBufferView* getBuffer();
		static Diligent::IBufferView* getZBinBuffer();
		static Diligent::IBufferView* getBoundsBuffer();

		static const rawrbox::Decal& get(size_t indx);
		static size_t count();
		// ----------

		// DECALS ----
		static void add(const rawrbox::Decal& decal);
		static bool remove(size_t indx);
		static void clear();
		// ---------
	};
} // namespace rawrbox
