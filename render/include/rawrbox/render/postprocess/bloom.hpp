#pragma once
#include <rawrbox/render/postprocess/base.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/util/uniforms.hpp>

#include <generated/shaders/render/all.hpp>

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <memory>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader bloom_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_post_bloom),
    BGFX_EMBEDDED_SHADER(fs_post_bloom),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrBox {
	class PostProcessBloom : public rawrBox::PostProcessBase {
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _bloom_intensity = BGFX_INVALID_HANDLE;

	protected:
		float _intensity = 0.08f;

	public:
		PostProcessBloom(float intensity) : _intensity(intensity) {}

		PostProcessBloom(PostProcessBloom&&) = delete;
		PostProcessBloom& operator=(PostProcessBloom&&) = delete;
		PostProcessBloom(const PostProcessBloom&) = delete;
		PostProcessBloom& operator=(const PostProcessBloom&) = delete;

		~PostProcessBloom() override {
			RAWRBOX_DESTROY(this->_program);
			RAWRBOX_DESTROY(this->_bloom_intensity);
		}

		virtual void setIntensity(float in) {
			this->_intensity = in;
			if (bgfx::isValid(this->_bloom_intensity)) rawrBox::UniformUtils::setUniform(this->_bloom_intensity, this->_intensity);
		}

		void upload() override {
			// Load Shader --------
			bgfx::RendererType::Enum type = bgfx::getRendererType();
			bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(bloom_shaders, type, "vs_post_bloom");
			bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(bloom_shaders, type, "fs_post_bloom");

			this->_program = bgfx::createProgram(vsh, fsh, true);
			if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-Bloom] Failed to initialize shader program");
			// ------------------

			this->_bloom_intensity = bgfx::createUniform("u_intensity", bgfx::UniformType::Vec4, 1);
			rawrBox::UniformUtils::setUniform(this->_bloom_intensity, this->_intensity);
		}

		void applyEffect() override {
			bgfx::submit(rawrBox::CURRENT_VIEW_ID, this->_program);
		}
	};
} // namespace rawrBox
