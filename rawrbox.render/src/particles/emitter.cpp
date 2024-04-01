#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/particles/emitter.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// PIPELINE ---
	bool Emitter::_PIPELINE_INIT = false;
	void Emitter::createPipeline() {
		if (Emitter::_PIPELINE_INIT) return;

		rawrbox::PipeComputeSettings settings;
		settings.signature = rawrbox::BindlessManager::computeSignature;

		// BUILDING -----
		settings.pCS = "particles.csh";
		rawrbox::PipelineUtils::createComputePipeline("Particles::Process", settings);
		// ---------
	}
	// --------------

	Emitter::Emitter(uint32_t maxParticles) : _maxParticles(maxParticles) {}
	Emitter::~Emitter() {
		RAWRBOX_DESTROY(this->_constants);

		this->_bufferRead = nullptr;
		this->_bufferWrite = nullptr;
		RAWRBOX_DESTROY(this->_buffer);
	}

	void Emitter::upload() {
		if (this->_constants != nullptr || this->_buffer != nullptr) throw this->_logger->error("Emitter already uploaded");
		this->_particles.reserve(this->_maxParticles);

		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Emitter::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DYNAMIC; // TODO: DEFAULT
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::EmitterUniforms);

			Diligent::BufferData data;
			data.pData = &this->_uniforms;
			data.DataSize = BuffDesc.Size;

			rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, nullptr, &this->_constants);
		}
		// -----------------------------------------

		// Create data --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::Particle);
			BuffDesc.Name = "RawrBox::Emitter::Buffer";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_UNORDERED_ACCESS;
			BuffDesc.Size = BuffDesc.ElementByteStride * this->_particles.capacity();

			rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, nullptr, &this->_buffer);

			_bufferWrite = this->_buffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS);
			_bufferRead = this->_buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// -----------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{this->_constants, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------

		// Setup pipeline --
		createPipeline();
		//----------------
	}

	// UTILS ----

	void Emitter::setPos(const rawrbox::Vector3f& pos) { this->_uniforms.position = pos; }
	const rawrbox::Vector3f& Emitter::getPos() const { return this->_uniforms.position; }

	void Emitter::setVelocity(const rawrbox::Vector3f& vel) { this->_uniforms.velocity = vel; }
	const rawrbox::Vector3f& Emitter::getVelocity() const { return this->_uniforms.velocity; }

	void Emitter::setMaxLifetime(float lifetime) { this->_uniforms.maxLifeTime = lifetime; }
	float Emitter::getMaxLifetime() const { return this->_uniforms.maxLifeTime; }

	Diligent::IBufferView* Emitter::getBuffer(bool readOnly) { return readOnly ? this->_bufferRead : this->_bufferWrite; }
	// --------

	void Emitter::updateConstants() {
		if (this->_constants == nullptr) throw this->_logger->error("Emitter not uploaded");

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::EmitterUniforms> Constants(rawrbox::RENDERER->context(), this->_constants, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		if (Constants == nullptr) throw _logger->error("Failed to map emitter constants buffer!");
		std::memcpy(Constants, &this->_uniforms, sizeof(rawrbox::EmitterUniforms));

		Constants->data.x = this->_particles.size();
		Constants->deltaTime = rawrbox::DELTA_TIME;
		// -----------
	}

	void Emitter::update() {
		auto* context = rawrbox::RENDERER->context();

		// TODO: MOVE ME TO UPDATE ONCE?
		this->updateConstants();
		// -----------------------------

		context->SetPipelineState(this->_process);
		context->DispatchCompute({rawrbox::MathUtils::divideRound<uint32_t>(this->_particles.size(), 255), 1, 1});
	}

	void Emitter::draw() {}
} // namespace rawrbox
