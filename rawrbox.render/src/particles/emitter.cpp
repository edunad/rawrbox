#include <rawrbox/render/particles/emitter.hpp>
#include <rawrbox/render/plugins/particle_engine.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

namespace rawrbox {
	EmitterBase::EmitterBase(uint32_t maxParticles) : _maxParticles(maxParticles) { this->setTexture(nullptr); }

	EmitterBase::~EmitterBase() {
		this->_bufferRead = nullptr;
		this->_bufferWrite = nullptr;
		RAWRBOX_DESTROY(this->_computeBind);
		RAWRBOX_DESTROY(this->_drawBind);

		RAWRBOX_DESTROY(this->_buffer);
		RAWRBOX_DESTROY(this->_uniformBuffer);

		// UNREGISTER ----
		auto* engine = rawrbox::RENDERER->getPlugin<rawrbox::ParticleEnginePlugin>("ParticleEngine");
		if (engine != nullptr) engine->unregisterEmitter(this);
		// ---------------------
	}

	void EmitterBase::upload() {
		if (this->_buffer != nullptr) RAWRBOX_CRITICAL("Emitter already uploaded");

		auto* engine = rawrbox::RENDERER->getPlugin<rawrbox::ParticleEnginePlugin>("ParticleEngine");
		if (engine == nullptr) RAWRBOX_CRITICAL("Emitter requires the `ParticleEngine` renderer plugin");

		// Create data --
		Diligent::BufferDesc BuffDesc;
		BuffDesc.ElementByteStride = sizeof(rawrbox::Particle);
		BuffDesc.Name = "RawrBox::Emitter::Buffer";
		BuffDesc.Usage = Diligent::USAGE_DEFAULT;
		BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
		BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;
		BuffDesc.Size = BuffDesc.ElementByteStride * this->_maxParticles;

		const std::vector<rawrbox::Particle> particles(this->_maxParticles);

		Diligent::BufferData BuffData;
		BuffData.pData = particles.data();
		BuffData.DataSize = BuffDesc.Size;

		rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, &BuffData, &this->_buffer);

		this->_bufferWrite = this->_buffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS);
		this->_bufferRead = this->_buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		// -----------

		// Create uniforms --
		Diligent::BufferDesc UniformDesc;
		UniformDesc.Name = "RawrBox::Emitter::Uniforms";
		UniformDesc.Usage = Diligent::USAGE_DEFAULT;
		UniformDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		UniformDesc.Size = sizeof(rawrbox::EmitterUniforms);

		rawrbox::RENDERER->device()->CreateBuffer(UniformDesc, nullptr, &this->_uniformBuffer);
		if (this->_uniformBuffer == nullptr) RAWRBOX_CRITICAL("Failed to create emitter uniforms");
		// -----------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({
		    {this->_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_uniformBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		// -----------

		// Bindings, set once ---
		engine->getSignature(false)->CreateShaderResourceBinding(&this->_computeBind, true);
		engine->getSignature(true)->CreateShaderResourceBinding(&this->_drawBind, true);
		if (this->_computeBind == nullptr || this->_drawBind == nullptr) RAWRBOX_CRITICAL("Failed to create emitter bindings");

		this->_computeBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Particles")->Set(this->_bufferWrite);
		this->_computeBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "EmitterConstants")->Set(this->_uniformBuffer);

		this->_drawBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Particles")->Set(this->_bufferRead);
		this->_drawBind->GetVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "EmitterConstants")->Set(this->_uniformBuffer);
		// -----------

		// Initialize material ----
		this->getMaterial()->init();
		// ------------

		// Register ---
		engine->registerEmitter(this);
		// ------------
	}

	void EmitterBase::draw() {
		if (!this->isEnabled()) return;
		if (this->_drawBind == nullptr) RAWRBOX_CRITICAL("Emitter not uploaded");

		auto* context = rawrbox::RENDERER->context();
		this->_uniforms.time += rawrbox::DELTA_TIME;

		// Bind pipeline ----
		context->SetPipelineState(this->getMaterial()->base);
		context->CommitShaderResources(this->_drawBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		// ----------

		Diligent::DrawAttribs DrawAttrs;
		DrawAttrs.NumVertices = this->_maxParticles;
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
		context->Draw(DrawAttrs);
	}
} // namespace rawrbox
