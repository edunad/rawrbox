#include <rawrbox/render/passes/clustered_light.hpp>

// https://github.com/DiligentGraphics/DiligentSamples/blob/master/Tutorials/Tutorial19_RenderPasses/src/Tutorial19_RenderPasses.cpp
namespace rawrbox {
	const std::string ClusteredLightPass::getID() const { return "Clustered::Light"; }

	void ClusteredLightPass::initialize(const rawrbox::Vector2i& renderSize) {
	}

	void ClusteredLightPass::resize(const rawrbox::Vector2i& renderSize) {
	}

	void ClusteredLightPass::render(const rawrbox::DrawPass& pass) {
	}

	ClusteredLightPass::~ClusteredLightPass() {
	}
} // namespace rawrbox
