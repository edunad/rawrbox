
#include <rawrbox/render/postprocess/bloom.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>
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
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(bloom_shaders, type, "vs_post_bloom");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(bloom_shaders, type, "fs_post_bloom");

		this->_program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-Bloom] Failed to initialize shader program");
		// ------------------

		this->_bloom_intensity = bgfx::createUniform("u_intensity", bgfx::UniformType::Vec4, 1);
		rawrbox::UniformUtils::setUniform(this->_bloom_intensity, this->_intensity);
	}

	void PostProcessBloom::applyEffect() {
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
	}
} // namespace rawrbox
