
#include <rawrbox/render/postprocess/static_noise.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/uniforms.hpp>
#include <rawrbox/utils/time.hpp>

#include <generated/shaders/render/all.hpp>

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
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(noise_shaders, type, "vs_post_noise");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(noise_shaders, type, "fs_post_noise");

		this->_program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-Noise] Failed to initialize shader program");
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
