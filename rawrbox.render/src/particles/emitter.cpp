#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/particles/emitter.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {

	// PIPELINE ---
	bool Emitter::_PIPELINE_INIT = false;
	void Emitter::createPipeline() {
		if (Emitter::_PIPELINE_INIT) return;
		Emitter::_PIPELINE_INIT = true;
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
			BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::Vector4u);

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
	Diligent::IBufferView* Emitter::getBuffer(bool readOnly) { return readOnly ? this->_bufferRead : this->_bufferWrite; }
	// --------

	void Emitter::updateConstants() {
		if (this->_constants == nullptr) throw this->_logger->error("Emitter not uploaded");
	}

	void Emitter::update() {
		auto* context = rawrbox::RENDERER->context();

		// context->SetPipelineState(this->_clusterBuildingComputeProgram);
		context->DispatchCompute({rawrbox::MathUtils::divideRound<uint32_t>(this->_particles.size(), 255), 1, 1});
	}

	void Emitter::draw() {}
} // namespace rawrbox
