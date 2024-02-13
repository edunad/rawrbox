#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/decals/decal.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <DynamicBuffer.hpp>

#include <memory>

namespace rawrbox {

	struct DecalsConstants {
		uint32_t total = 0;
	};

	struct DecalVertex {
		rawrbox::Matrix4x4 worldToLocal = {};
		rawrbox::Vector4_t<uint32_t> data = {};
		rawrbox::Colorf color = {};
	};

	class DECALS {
	protected:
		static std::vector<std::unique_ptr<rawrbox::Decal>> _decals;

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
		static Diligent::IBufferView* getBuffer();

		static rawrbox::Decal* get(size_t indx);
		static size_t count();
		// ----------

		// DECALS ----
		static void add(const rawrbox::Decal& decal);

		static bool remove(size_t indx);
		static bool remove(const rawrbox::Decal& decal);
		// ---------
	};
} // namespace rawrbox
