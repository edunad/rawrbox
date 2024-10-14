#pragma once

#include <rawrbox/render/materials/particle.hpp>
#include <rawrbox/render/plugins/base.hpp>

namespace rawrbox {
	template <typename M = rawrbox::MaterialParticle>
		requires(std::derived_from<M, rawrbox::MaterialParticle>)
	class Emitter;

	class ParticleEnginePlugin : public rawrbox::RenderPlugin {
	protected:
		std::vector<rawrbox::Emitter<>*> _registeredEmitters = {};

		Diligent::IPipelineState* _updateProgram = nullptr;

		// SIGNATURE ---
		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> _signature;
		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> _dynamicSignature;

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _signatureBind;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _dynamicSignatureBind;
		// --------------

		// BUFFERS -----
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		// -------------

		void createSignatures();
		void createBuffers();

		void createPipelines();

		// RENDERING ---
		void setupEmitter(rawrbox::Emitter<>* emitter);
		void computeEmitter(rawrbox::Emitter<>* emitter);
		// ---------

	public:
		ParticleEnginePlugin() = default;
		ParticleEnginePlugin(const ParticleEnginePlugin&) = delete;
		ParticleEnginePlugin(ParticleEnginePlugin&&) = delete;
		ParticleEnginePlugin& operator=(const ParticleEnginePlugin&) = delete;
		ParticleEnginePlugin& operator=(ParticleEnginePlugin&&) = delete;
		~ParticleEnginePlugin() override;

		void initialize(const rawrbox::Vector2u& size) override;
		void upload() override;

		void signatures(std::vector<Diligent::PipelineResourceDesc>& sig) override;
		void bindStatic(Diligent::IPipelineResourceSignature& sig) override;

		void preRender(const rawrbox::CameraBase& camera) override;

		[[nodiscard]] Diligent::IPipelineResourceSignature* getSignature(bool dynamic = true) const;
		[[nodiscard]] Diligent::IShaderResourceBinding* getBind(bool dynamic = true) const;

		// REGISTER ----
		void registerEmitter(rawrbox::Emitter<>* em);
		void unregisterEmitter(rawrbox::Emitter<>* em);
		// -------------

		std::string getID() override;
	};
} // namespace rawrbox
