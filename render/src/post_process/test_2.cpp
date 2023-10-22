
#include <rawrbox/render/post_process/test_2.hpp>

namespace rawrbox {
	void Test2::upload(Diligent::IRenderPass* pass, uint32_t passIndex) {
		/*// Load Shader --------
		rawrbox::RenderUtils::buildShader(bloom_shaders, this->_program);
		// ------------------

		this->_bloom_intensity = bgfx::createUniform("u_intensity", bgfx::UniformType::Vec4, 1);
		rawrbox::UniformUtils::setUniform(this->_bloom_intensity, this->_intensity);*/
	}

	void Test2::setRTTexture(Diligent::ITextureView* texture) {
		if (auto* pInputColor = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_SubpassInputColor"))
			pInputColor->Set(texture);
	}

	void Test2::applyEffect() {
		// bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
	}
} // namespace rawrbox
