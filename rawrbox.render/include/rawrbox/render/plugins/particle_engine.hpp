#pragma once

#include <rawrbox/render/plugins/base.hpp>

namespace rawrbox {
	class EmitterBase;

	class ParticleEnginePlugin : public rawrbox::RenderPlugin {
	protected:
		std::vector<rawrbox::EmitterBase*> _registeredEmitters = {};

		Diligent::IPipelineState* _updateProgram = nullptr;

		// SIGNATURE ---
		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> _signature;
		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> _drawSignature;
		// --------------

		void createSignatures();

		void createPipelines();

		// RENDERING ---
		void setupEmitter(rawrbox::EmitterBase* emitter);
		void computeEmitter(rawrbox::EmitterBase* emitter);
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

		void preRender(const rawrbox::CameraBase& camera) override;

		[[nodiscard]] Diligent::IPipelineResourceSignature* getSignature(bool draw = true) const;

		// REGISTER ----
		void registerEmitter(rawrbox::EmitterBase* em);
		void unregisterEmitter(rawrbox::EmitterBase* em);
		// -------------

		std::string getID() override;
	};
} // namespace rawrbox
