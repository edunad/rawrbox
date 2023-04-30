#pragma once
#include <rawrbox/render/postprocess/base.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/util/uniforms.hpp>
#include <rawrbox/utils/time.hpp>

#include <generated/shaders/render/all.hpp>

#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <bx/timer.h>

#include <algorithm>
#include <memory>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader noise_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_post_noise),
    BGFX_EMBEDDED_SHADER(fs_post_noise),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrBox {
	class PostProcessStaticNoise : public rawrBox::PostProcessBase {
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _settings = BGFX_INVALID_HANDLE;

		int64_t m_timeOffset = 0;
		float _strength = 0.1F;

	protected:
	public:
		explicit PostProcessStaticNoise(float s) : _strength(std::clamp(s, 0.F, 1.F)){};
		PostProcessStaticNoise(PostProcessStaticNoise&&) = delete;
		PostProcessStaticNoise& operator=(PostProcessStaticNoise&&) = delete;
		PostProcessStaticNoise(const PostProcessStaticNoise&) = delete;
		PostProcessStaticNoise& operator=(const PostProcessStaticNoise&) = delete;

		~PostProcessStaticNoise() override {
			RAWRBOX_DESTROY(this->_program);
			RAWRBOX_DESTROY(this->_settings);
		}

		void setStrength(float s) {
			this->_strength = std::clamp(s, 0.F, 1.F);
		}

		void upload() override {
			// Load Shader --------
			bgfx::RendererType::Enum type = bgfx::getRendererType();
			bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(noise_shaders, type, "vs_post_noise");
			bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(noise_shaders, type, "fs_post_noise");

			this->_program = bgfx::createProgram(vsh, fsh, true);
			if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-Noise] Failed to initialize shader program");
			// ------------------

			this->_settings = bgfx::createUniform("u_settings", bgfx::UniformType::Vec4, 2);
			rawrBox::UniformUtils::setUniform(this->_settings, rawrBox::Vector2f(0,
									       this->_strength));

			m_timeOffset = bx::getHPCounter();
		}

		void applyEffect() override {
			int64_t now = bx::getHPCounter();
			const auto freq = double(bx::getHPFrequency());
			auto time = (float)((now - m_timeOffset) / freq);

			rawrBox::UniformUtils::setUniform(this->_settings, rawrBox::Vector2f(time,
									       this->_strength));
			bgfx::submit(rawrBox::CURRENT_VIEW_ID, this->_program);
		}
	};
} // namespace rawrBox
