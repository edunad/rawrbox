
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/post_process/quick_bloom.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	PostProcessQuickBloom::PostProcessQuickBloom(float intensity) {
		this->setIntensity(intensity);
	}

	void PostProcessQuickBloom::setIntensity(float in) {
		this->_data[0].x = in; // BloomIntensity
	}

	void PostProcessQuickBloom::init() {
		rawrbox::PostProcessBase::init();
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Bloom requires a main camera!");

		// TEXTURES ----
		auto* cameraTarget = rawrbox::MAIN_CAMERA->getRenderTarget();
		auto format = cameraTarget->getTexture()->GetDesc().Format;

		const auto& size = cameraTarget->getSize();
		rawrbox::Vector2u halfSize = {std::max(size.x / 2U, 1U), std::max(size.y / 2U, 1U)};

		this->_bloomA = std::make_unique<rawrbox::TextureRender>(halfSize, false);
		this->_bloomA->setName("PostProcess::Bloom::A");
		this->_bloomA->upload(format);

		this->_bloomB = std::make_unique<rawrbox::TextureRender>(halfSize, false);
		this->_bloomB->setName("PostProcess::Bloom::B");
		this->_bloomB->upload(format);
		// ----------

		// PIPELINES ----
		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.depthWrite = false;
		settings.depthFormat = false;
		settings.blending = {.src = Diligent::BLEND_FACTOR_UNDEFINED, .dest = Diligent::BLEND_FACTOR_UNDEFINED};
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "quad.vsh";
		settings.pPS = "quick_bloom.psh";
		settings.signatures = {rawrbox::BindlessManager::signature};

		settings.macros.Add("BLOOM_PREFILTER", true);
		this->_preFilter = rawrbox::PipelineUtils::createPipeline("PostProcess::QuickBloom::Prefilter", settings);

		settings.macros.Clear();
		settings.macros.Add("BLOOM_BLUR", true);
		this->_blur = rawrbox::PipelineUtils::createPipeline("PostProcess::QuickBloom::Blur", settings);

		settings.macros.Clear();
		this->_pipeline = rawrbox::PipelineUtils::createPipeline("PostProcess::QuickBloom", settings);
		// ----------
	}

	void PostProcessQuickBloom::applyEffect(const rawrbox::TextureBase& source, const rawrbox::TextureRender& target, uint32_t depthID) {
		if (!source.isValid()) RAWRBOX_CRITICAL("Effect texture not uploaded!");

		const auto& size = source.getSize();
		const auto& halfSize = this->_bloomA->getSize();

		rawrbox::Vector2f texel = {1.F / static_cast<float>(size.x), 1.F / static_cast<float>(size.y)};
		rawrbox::Vector2f halfTexel = {1.F / static_cast<float>(halfSize.x), 1.F / static_cast<float>(halfSize.y)};

		// PREFILTER ---
		this->_data[1] = {0.F, 0.F, texel.x, texel.y};
		this->draw(this->_preFilter, *this->_bloomA, {source.getTextureID(), 0, 0, 0});
		// ----------

		// BLUR ---
		this->_data[1] = {halfTexel.x, 0.F, halfTexel.x, halfTexel.y};
		this->draw(this->_blur, *this->_bloomB, {this->_bloomA->getTextureID(), 0, 0, 0});

		this->_data[1] = {0.F, halfTexel.y, halfTexel.x, halfTexel.y};
		this->draw(this->_blur, *this->_bloomA, {this->_bloomB->getTextureID(), 0, 0, 0});
		// ----------

		// COMBINE ---
		this->_data[1] = {0.F, 0.F, halfTexel.x, halfTexel.y};
		this->draw(this->_pipeline, target, {source.getTextureID(), depthID, this->_bloomA->getTextureID(), 0});
		// ----------
	}
} // namespace rawrbox