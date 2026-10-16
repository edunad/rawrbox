#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/particles/emitter.hpp>
#include <rawrbox/render/plugins/particle_engine.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	ParticleEnginePlugin::~ParticleEnginePlugin() {
		RAWRBOX_DESTROY(this->_signature);
		RAWRBOX_DESTROY(this->_drawSignature);
	}

	void ParticleEnginePlugin::initialize(const rawrbox::Vector2u& /*size*/) {
		this->createSignatures();
	}

	void ParticleEnginePlugin::createSignatures() {
		if (this->_signature != nullptr || this->_drawSignature != nullptr) RAWRBOX_CRITICAL("Signatures already bound!");

		std::vector<Diligent::PipelineResourceDesc> resources = {
		    // CAMERA ------
		    {Diligent::SHADER_TYPE_COMPUTE, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_COMPUTE, "SCamera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    // --------------

		    // PARTICLES ---
		    {Diligent::SHADER_TYPE_COMPUTE, "EmitterConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_COMPUTE, "Particles", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    // ------------
		};

		// Compute signature ---
		Diligent::PipelineResourceSignatureDesc PRSDesc;
		PRSDesc.Name = "RawrBox::SIGNATURE::Particles";
		PRSDesc.BindingIndex = 0;

		PRSDesc.ImmutableSamplers = nullptr;
		PRSDesc.NumImmutableSamplers = 0;

		PRSDesc.Resources = resources.data();
		PRSDesc.NumResources = static_cast<uint8_t>(resources.size());

		rawrbox::RENDERER->device()->CreatePipelineResourceSignature(PRSDesc, &this->_signature);
		// ----------------------

		// Draw signature ---
		PRSDesc.Name = "RawrBox::SIGNATURE::Particles::Draw";
		PRSDesc.BindingIndex = 1;

		resources = {
		    {Diligent::SHADER_TYPE_VERTEX, "Particles", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_GEOMETRY, "EmitterConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		};

		PRSDesc.Resources = resources.data();
		PRSDesc.NumResources = static_cast<uint8_t>(resources.size());

		rawrbox::RENDERER->device()->CreatePipelineResourceSignature(PRSDesc, &this->_drawSignature);
		// ----------------
	}

	void ParticleEnginePlugin::createPipelines() {
		rawrbox::PipeComputeSettings settings;
		settings.signatures = {this->_signature};

		// PROCESS -----
		settings.pCS = "plugins/particle_engine/particles.csh";
		this->_updateProgram = rawrbox::PipelineUtils::createComputePipeline("ParticleEngine::Process", settings);
		// ---------
	}

	void ParticleEnginePlugin::upload() {
		if (this->_signature == nullptr || this->_drawSignature == nullptr) RAWRBOX_CRITICAL("Plugin not initialized!");

		// Compute bind ---
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Camera")->Set(rawrbox::CameraBase::uniforms);
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);
		// ----------------

		this->createPipelines();
	}

	void ParticleEnginePlugin::preRender(const rawrbox::CameraBase& camera) {
		if (&camera != rawrbox::MAIN_CAMERA) return; // Simulate once per frame

		for (auto* emitter : this->_registeredEmitters) {
			if (emitter == nullptr || !emitter->isEnabled()) continue;

			this->setupEmitter(emitter);
			this->computeEmitter(emitter);
		}
	}

	Diligent::IPipelineResourceSignature* ParticleEnginePlugin::getSignature(bool draw) const { return draw ? this->_drawSignature : this->_signature; }

	// REGISTER ----
	void ParticleEnginePlugin::registerEmitter(rawrbox::Emitter<>* em) {
		this->_registeredEmitters.push_back(em);
	}

	void ParticleEnginePlugin::unregisterEmitter(rawrbox::Emitter<>* em) {
		for (auto it = this->_registeredEmitters.begin(); it != this->_registeredEmitters.end();) {
			if ((*it) != nullptr && (*it) == em) {
				it = this->_registeredEmitters.erase(it);
				continue;
			}

			++it;
		}
	}
	// -------------

	// RENDERING ---
	void ParticleEnginePlugin::setupEmitter(rawrbox::Emitter<>* emitter) {
		if (emitter == nullptr) RAWRBOX_CRITICAL("Invalid emitter");
		if (this->_signature == nullptr) RAWRBOX_CRITICAL("Plugin not uploaded!");

		auto* uniforms = emitter->getUniformBuffer();

		// Update uniforms ----
		rawrbox::RENDERER->context()->UpdateBuffer(uniforms, 0, sizeof(rawrbox::EmitterUniforms), &emitter->getUniform(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------
	}

	void ParticleEnginePlugin::computeEmitter(rawrbox::Emitter<>* emitter) {
		if (emitter == nullptr) RAWRBOX_CRITICAL("Invalid emitter");
		if (this->_signature == nullptr) RAWRBOX_CRITICAL("Plugin not uploaded!");

		auto* renderer = rawrbox::RENDERER;
		auto* context = renderer->context();
		auto* buffer = emitter->getData();

		// Barrier for writting -----
		rawrbox::BarrierUtils::barrier({
		    {buffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		//  -----------

		// Update the particles ---
		context->SetPipelineState(this->_updateProgram);
		context->CommitShaderResources(emitter->getBind(false), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		context->DispatchCompute({rawrbox::MathUtils::divideRound<uint32_t>(emitter->maxParticles(), 256), 1, 1});
		/// ------------

		// Barrier for reading -----
		rawrbox::BarrierUtils::barrier({
		    {buffer, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		//  -----------
	}
	// ---------------------

	std::string ParticleEnginePlugin::getID() { return "ParticleEngine"; }
	// ----
} // namespace rawrbox
