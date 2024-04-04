#pragma once

#include <rawrbox/render/materials/particle.hpp>
#include <rawrbox/render/particles/particle.hpp>
#include <rawrbox/render/plugins/particle_engine.hpp>
#include <rawrbox/utils/logger.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>
#include <PipelineState.h>

namespace rawrbox {

	struct EmitterUniforms {
	public:
		rawrbox::Vector3f position = {};
		float time = 0.F;

		std::array<rawrbox::Vector4f, 2> velocity = {}; // Random between 2 values
		std::array<rawrbox::Colorf, 4> color = {};      // Transition between 4 colors using lifetime: 0 ---- 1 ---- 2 ----> 3

		// -----
		rawrbox::Vector4f size = {}; // Random between 2 values
					     // -------

		// -----
		rawrbox::Vector2f life = {};
		float spawnRate = 0;
		float maxParticles = 0;
		// -------
	};

	template <typename M>
		requires(std::derived_from<M, rawrbox::MaterialParticle>)
	class Emitter {
	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _buffer;
		Diligent::IBufferView* _bufferRead = nullptr;
		Diligent::IBufferView* _bufferWrite = nullptr;

		rawrbox::EmitterUniforms _uniforms = {};
		std::unique_ptr<M> _material = std::make_unique<M>();

		uint32_t _maxParticles = 0;
		bool _enabled = true;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Emitter");
		// -------------

	public:
		Emitter(uint32_t maxParticles) : _maxParticles(maxParticles) {}
		Emitter(const Emitter&) = delete;
		Emitter(Emitter&&) = delete;
		Emitter& operator=(const Emitter&) = delete;
		Emitter& operator=(Emitter&&) = delete;
		virtual ~Emitter() {
			this->_bufferRead = nullptr;
			this->_bufferWrite = nullptr;
			RAWRBOX_DESTROY(this->_buffer);

			// UNREGISTER ----
			auto* engine = rawrbox::RENDERER->getPlugin<rawrbox::ParticleEnginePlugin>("ParticleEngine");
			if (engine != nullptr) engine->unregisterEmitter(this);
			// ---------------------
		}

		// UTILS ----
		virtual void setPos(const rawrbox::Vector3f& pos) { this->_uniforms.position = pos; }
		[[nodiscard]] virtual rawrbox::Vector3f getPos() const { return this->_uniforms.position; }

		[[nodiscard]] virtual const rawrbox::Vector4f& getSize() const { return this->_uniforms.size; }
		virtual void setSize(const rawrbox::Vector4f& size) { this->_uniforms.size = size; }

		[[nodiscard]] virtual std::array<rawrbox::Vector4f, 2> getVelocity() const { return this->_uniforms.velocity; }
		virtual void setVelocity(const rawrbox::Vector3f& min, const rawrbox::Vector3f& max) {
			this->_uniforms.velocity[0] = min;
			this->_uniforms.velocity[1] = max;
		}

		virtual void setColorTransition(const std::array<rawrbox::Colorf, 4>& col) { this->_uniforms.color = col; }
		[[nodiscard]] virtual const std::array<rawrbox::Colorf, 4>& getColorTransition() const { return this->_uniforms.color; }

		virtual void setLifetimeRange(float minLife, float maxLife) { this->_uniforms.life = {minLife, maxLife}; }
		[[nodiscard]] virtual const rawrbox::Vector2f& getLifetimeRange() const { return this->_uniforms.life; }

		virtual void setSpawnRate(float rate) { this->_uniforms.spawnRate = rate; }
		[[nodiscard]] virtual float getSpawnRate() const { return this->_uniforms.spawnRate; }

		virtual void setEnabled(bool enabled) { this->_enabled = enabled; }
		[[nodiscard]] virtual bool isEnabled() const { return this->_enabled; }

		[[nodiscard]] virtual Diligent::IBufferView* getBuffer(bool readOnly = true) const { return readOnly ? this->_bufferRead : this->_bufferWrite; }
		[[nodiscard]] virtual Diligent::IBuffer* getData() const { return this->_buffer; }

		[[nodiscard]] virtual const rawrbox::EmitterUniforms& getUniform() const { return this->_uniforms; }

		[[nodiscard]] virtual uint32_t maxParticles() const { return this->_maxParticles; }
		// --------

		virtual void upload() {
			if (this->_buffer != nullptr) throw this->_logger->error("Emitter already uploaded");

			auto* engine = rawrbox::RENDERER->getPlugin<rawrbox::ParticleEnginePlugin>("ParticleEngine");
			if (engine == nullptr) throw this->_logger->error("Emitter requires the `ParticleEngine` renderer plugin");

			// Create data --
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::Particle);
			BuffDesc.Name = "RawrBox::Emitter::Buffer";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;
			BuffDesc.Size = BuffDesc.ElementByteStride * this->_maxParticles;

			rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, nullptr, &this->_buffer);

			_bufferWrite = this->_buffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS);
			_bufferRead = this->_buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
			// -----------

			// BARRIER -----
			rawrbox::BarrierUtils::barrier({{this->_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			// -----------

			// Initialize material ----
			this->_material->init();
			// ------------

			// Register ---
			engine->registerEmitter(this);
			// ------------
		}

		virtual void draw() {
			if (!this->isEnabled()) return;
			this->_uniforms.time++;

			auto* context = rawrbox::RENDERER->context();

			// Setup bind ---
			rawrbox::BindlessManager::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Particles")->Set(this->_bufferRead /*, Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE*/);
			// ---------------

			context->CommitShaderResources(rawrbox::BindlessManager::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			// Bind pipeline ----
			context->SetPipelineState(this->_material->base);
			// ----------

			Diligent::DrawAttribs DrawAttrs;
			DrawAttrs.NumVertices = this->_maxParticles * 6; // 4 vertices per particle quad
			DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
			context->Draw(DrawAttrs);
		}
	};
} // namespace rawrbox
