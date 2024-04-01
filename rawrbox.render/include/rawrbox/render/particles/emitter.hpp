#pragma once

#include <rawrbox/render/particles/particle.hpp>
#include <rawrbox/utils/logger.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>
#include <PipelineState.h>

namespace rawrbox {

	struct EmitterUniforms {
	public:
		rawrbox::Vector3f position = {};
		float maxLifeTime = 0;

		rawrbox::Vector3f velocity = {};
		float deltaTime = 0;

		rawrbox::Vector4u data = {}; // TOTAL_PARTICLES, etc
	};

	class Emitter {
	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _constants;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _buffer;

		Diligent::IBufferView* _bufferRead = nullptr;
		Diligent::IBufferView* _bufferWrite = nullptr;

		Diligent::IPipelineState* _process = nullptr;

		rawrbox::EmitterUniforms _uniforms = {};

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
		virtual void setPos(const rawrbox::Vector3f& pos);
		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const;

		virtual void setVelocity(const rawrbox::Vector3f& vel);
		[[nodiscard]] virtual const rawrbox::Vector3f& getVelocity() const;

		virtual void setMaxLifetime(float lifetime);
		[[nodiscard]] virtual float getMaxLifetime() const;

		virtual Diligent::IBufferView* getBuffer(bool readOnly = true);
		// --------

		virtual void upload();
		virtual void update();
		virtual void draw();
	};
} // namespace rawrbox
