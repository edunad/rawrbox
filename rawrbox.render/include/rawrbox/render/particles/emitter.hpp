#pragma once

#include <rawrbox/render/particles/particle.hpp>
#include <rawrbox/utils/logger.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

namespace rawrbox {
	class Emitter {
	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _constants;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _buffer;

		Diligent::IBufferView* _bufferRead = nullptr;
		Diligent::IBufferView* _bufferWrite = nullptr;

		std::vector<rawrbox::Particle> _particles = {};
		uint32_t _maxParticles = 0;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Emitter");
		// -------------

		virtual void updateConstants();

		// PIPELINE ---
		static bool _PIPELINE_INIT;
		static void createPipeline();
		// --------------
	public:
		Emitter(uint32_t maxParticles);
		Emitter(const Emitter&) = delete;
		Emitter(Emitter&&) = delete;
		Emitter& operator=(const Emitter&) = delete;
		Emitter& operator=(Emitter&&) = delete;
		virtual ~Emitter();

		// UTILS ----
		virtual Diligent::IBufferView* getBuffer(bool readOnly = true);
		// --------

		virtual void upload();
		virtual void update();
		virtual void draw();
	};
} // namespace rawrbox
