
#include <rawrbox/render/postprocess/static_noise.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>
#include <rawrbox/render/utils/uniforms.hpp>
#include <rawrbox/utils/time.hpp>

#include <bx/math.h>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader noise_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_post_noise),
    BGFX_EMBEDDED_SHADER(fs_post_noise),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	PostProcessStaticNoise::PostProcessStaticNoise(float s) : _strength(std::clamp(s, 0.F, 1.F)){};
	PostProcessStaticNoise::~PostProcessStaticNoise() {
		RAWRBOX_DESTROY(this->_program);
		RAWRBOX_DESTROY(this->_settings);
	}

	void PostProcessStaticNoise::setStrength(float s) {
		this->_strength = std::clamp(s, 0.F, 1.F);
	}

	void PostProcessStaticNoise::upload() {
		// Load Shader --------
		rawrbox::RenderUtils::buildShader(noise_shaders, this->_program);
		// ------------------

		this->_settings = bgfx::createUniform("u_settings", bgfx::UniformType::Vec4, 2);
		rawrbox::UniformUtils::setUniform(this->_settings, rawrbox::Vector2f(0,
								       this->_strength));
	}

	void PostProcessStaticNoise::applyEffect() {
		rawrbox::UniformUtils::setUniform(this->_settings, rawrbox::Vector2f(rawrbox::TimeUtils::time(),
								       this->_strength));
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
	}
} // namespace rawrbox
