#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/decals/decal.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <DynamicBuffer.hpp>

namespace rawrbox {

	class DECALS {
	protected:
		static std::vector<rawrbox::Decal> _decals;

		static std::unique_ptr<Diligent::DynamicBuffer> _buffer;
		static Diligent::IBufferView* _bufferRead;

		static bool _CONSTANTS_DIRTY;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

		static void updateConstants();
		static void updateBuffer();

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		static void init();
		static void shutdown();

		static void update();

		// UTILS ----
		static Diligent::IBufferView* getBuffer();

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
