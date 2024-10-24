
#include <rawrbox/render/materials/particle.hpp>
#include <rawrbox/render/plugins/particle_engine.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialParticle::_built = false;
	// ----------------

	void MaterialParticle::init() {
		const std::string id = "Particle::Unlit";

		if (!rawrbox::MaterialParticle::_built) {
			this->_logger->info("Building {} material..", fmt::styled(id, fmt::fg(fmt::color::azure)));

			this->createPipelines(id, {});
			rawrbox::MaterialParticle::_built = true;
		}

		this->setupPipelines(id);
	}

	void MaterialParticle::createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& /*layout*/, const Diligent::ShaderMacroHelper& /*helper*/) {
		auto* engine = rawrbox::RENDERER->getPlugin<rawrbox::ParticleEnginePlugin>("ParticleEngine");
		if (engine == nullptr) RAWRBOX_CRITICAL("This material requires the `ParticleEngine` renderer plugin");
		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "particle.vsh";
		settings.pPS = "particle_unlit.psh";
		settings.pGS = "particle_unlit.gsh";
		settings.renderTargets = RB_RENDER_RENDER_TARGET_TARGETS;    // COLOR + GPUPick
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_POINT_LIST; // Set topology
		settings.cull = Diligent::CULL_MODE_NONE;
		// settings.depthWrite = false;
		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		settings.fill = Diligent::FILL_MODE_SOLID;
		settings.signatures = {rawrbox::BindlessManager::signature, engine->getSignature()};
		// -------------------

		rawrbox::PipelineUtils::createPipeline(id, settings);
		// -----
	}
} // namespace rawrbox
