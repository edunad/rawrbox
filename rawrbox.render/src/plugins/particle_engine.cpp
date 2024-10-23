#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/particles/emitter.hpp>
#include <rawrbox/render/plugins/particle_engine.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	ParticleEnginePlugin::~ParticleEnginePlugin() {
		RAWRBOX_DESTROY(this->_signature);
		RAWRBOX_DESTROY(this->_signatureBind);
		RAWRBOX_DESTROY(this->_uniforms);
	}

	void ParticleEnginePlugin::initialize(const rawrbox::Vector2u& /*size*/) {
		this->createBuffers();
		this->createSignatures();
	}

	void ParticleEnginePlugin::createSignatures() {
		if (this->_signature != nullptr || this->_signatureBind != nullptr) RAWRBOX_CRITICAL("Signatures already bound!");
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Clustered plugin requires at least one camera!");

		std::vector<Diligent::PipelineResourceDesc> resources = {
		    // CAMERA ------
		    {Diligent::SHADER_TYPE_COMPUTE, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_COMPUTE, "SCamera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    // --------------

		    // PARTICLES ---
		    {Diligent::SHADER_TYPE_COMPUTE, "EmitterConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "Particles", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
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

		// Dynamic signature ---
		PRSDesc.Name = "RawrBox::SIGNATURE::Particles::Dynamic";
		PRSDesc.BindingIndex = 1;

		resources = {
		    {Diligent::SHADER_TYPE_VERTEX, "Particles", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		};

		PRSDesc.Resources = resources.data();
		PRSDesc.NumResources = static_cast<uint8_t>(resources.size());

		rawrbox::RENDERER->device()->CreatePipelineResourceSignature(PRSDesc, &this->_dynamicSignature);
		// ----------------
	}

	void ParticleEnginePlugin::createBuffers() {
		Diligent::BufferDesc BuffDesc;
		BuffDesc.Name = "rawrbox::Emitter::Uniforms";
		BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
		BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		BuffDesc.Size = sizeof(rawrbox::EmitterUniforms);

		rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, nullptr, &this->_uniforms);

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{this->_uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------
	}

	void ParticleEnginePlugin::createPipelines() {
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Particle engine plugin requires at least one camera!");

		rawrbox::PipeComputeSettings settings;
		settings.signatures = {this->_signature};

		// PROCESS -----
		settings.pCS = "particles.csh";
		this->_updateProgram = rawrbox::PipelineUtils::createComputePipeline("ParticleEngine::Process", settings);
		// ---------
	}

	void ParticleEnginePlugin::upload() {
		if (this->_signature == nullptr || this->_signatureBind == nullptr || this->_uniforms == nullptr) RAWRBOX_CRITICAL("Plugin not initialized!");

		// Compute bind ---
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Camera")->Set(rawrbox::CameraBase::uniforms);
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);

		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "EmitterConstants")->Set(this->_uniforms);

		this->_signature->CreateShaderResourceBinding(&this->_signatureBind, true);
		this->_dynamicSignature->CreateShaderResourceBinding(&this->_dynamicSignatureBind, true);
		// ----------------

		this->createPipelines();
	}

	void ParticleEnginePlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& sig) {
		sig.emplace_back(Diligent::SHADER_TYPE_GEOMETRY, "EmitterConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
	}

	void ParticleEnginePlugin::bindStatic(Diligent::IPipelineResourceSignature& sig) {
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "EmitterConstants")->Set(this->_uniforms);
	}

	void ParticleEnginePlugin::preRender(const rawrbox::CameraBase& /*camera*/) {
		for (auto* emitter : this->_registeredEmitters) {
			if (emitter == nullptr || !emitter->isEnabled()) continue;

			this->setupEmitter(emitter);
			this->computeEmitter(emitter);
		}
	}

	Diligent::IPipelineResourceSignature* ParticleEnginePlugin::getSignature(bool dynamic) const { return dynamic ? this->_dynamicSignature : this->_signature; }
	Diligent::IShaderResourceBinding* ParticleEnginePlugin::getBind(bool dynamic) const { return dynamic ? this->_dynamicSignatureBind : this->_signatureBind; }

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
		if (this->_signature == nullptr || this->_signatureBind == nullptr || this->_uniforms == nullptr) RAWRBOX_CRITICAL("Plugin not uploaded!");

		// Update uniforms ----
		{
			Diligent::MapHelper<rawrbox::EmitterUniforms> Constants(rawrbox::RENDERER->context(), this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			if (Constants == nullptr) RAWRBOX_CRITICAL("Failed to map emitter constants buffer!");

			std::memcpy(Constants, &emitter->getUniform(), sizeof(rawrbox::EmitterUniforms));
		}
		// --------

		// Setup signature ---
		this->_signatureBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Particles")->Set(emitter->getBuffer(false));
		//  ------------------
	}

	void ParticleEnginePlugin::computeEmitter(rawrbox::Emitter<>* emitter) {
		if (emitter == nullptr) RAWRBOX_CRITICAL("Invalid emitter");
		if (this->_signature == nullptr || this->_signatureBind == nullptr || this->_uniforms == nullptr) RAWRBOX_CRITICAL("Plugin not uploaded!");

		auto* renderer = rawrbox::RENDERER;
		auto* context = renderer->context();
		auto* buffer = emitter->getData();

		// Barrier for writting -----
		rawrbox::BarrierUtils::barrier({
		    {buffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		//  -----------

		// Commit compute signature --
		context->CommitShaderResources(this->_signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		// -----------------------

		// Update the particles ---
		context->SetPipelineState(this->_updateProgram);
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
