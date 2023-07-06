
#include <rawrbox/render/postprocess/bloom.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>
#include <rawrbox/render/utils/uniforms.hpp>

#include <bx/math.h>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader bloom_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_post_bloom),
    BGFX_EMBEDDED_SHADER(fs_post_bloom),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	PostProcessBloom::PostProcessBloom(float intensity) : _intensity(intensity) {}
	PostProcessBloom::~PostProcessBloom() {
		RAWRBOX_DESTROY(this->_program);
		RAWRBOX_DESTROY(this->_bloom_intensity);
	}

	void PostProcessBloom::setIntensity(float in) {
		this->_intensity = in;
		if (bgfx::isValid(this->_bloom_intensity)) {
			rawrbox::UniformUtils::setUniform(this->_bloom_intensity, this->_intensity);
		}
	}

	void PostProcessBloom::upload() {
		// Load Shader --------
		rawrbox::RenderUtils::buildShader(bloom_shaders, this->_program);
		// ------------------

		this->_bloom_intensity = bgfx::createUniform("u_intensity", bgfx::UniformType::Vec4, 1);
		rawrbox::UniformUtils::setUniform(this->_bloom_intensity, this->_intensity);
	}

	void PostProcessBloom::applyEffect() {
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
	}
} // namespace rawrbox
