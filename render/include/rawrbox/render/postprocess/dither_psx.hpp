#pragma once
#include <rawrbox/render/postprocess/base.hpp>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>
#include <rawrbox/render/texture/image.h>
#include <rawrbox/render/util/uniforms.hpp>

// Compiled shaders
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <generated/shaders/render/all.h>

#include <memory>
#include <unordered_map>

static const bgfx::EmbeddedShader shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_post_dither),
    BGFX_EMBEDDED_SHADER(fs_post_dither),
    BGFX_EMBEDDED_SHADER_END()};

namespace rawrBox {
	enum DITHER_SIZE {
		_SLOW_MODE = 0,

		_2x2 = 2,
		_3x3 = 3,
		_4x4 = 4,
		_8x8 = 8,
	};

	class PostProcessPSXDither : public rawrBox::PostProcessBase {
		std::unordered_map<DITHER_SIZE, std::shared_ptr<rawrBox::TextureImage>> _textures;
		DITHER_SIZE _size = DITHER_SIZE::_SLOW_MODE;

		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _ditherColor = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _dither_size = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _dither_color_depth = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _dither_dithering = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _dithering_intensity = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _dithering_depth = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _dither_threshold = BGFX_INVALID_HANDLE;

	protected:
	public:
		~PostProcessPSXDither() {
			this->_textures.clear();

			RAWRBOX_DESTROY(this->_program);

			RAWRBOX_DESTROY(this->_ditherColor);

			RAWRBOX_DESTROY(this->_dither_size);
			RAWRBOX_DESTROY(this->_dither_color_depth);
			RAWRBOX_DESTROY(this->_dither_dithering);
			RAWRBOX_DESTROY(this->_dithering_intensity);
			RAWRBOX_DESTROY(this->_dithering_depth);
			RAWRBOX_DESTROY(this->_dither_threshold);
		}

		PostProcessPSXDither(DITHER_SIZE dither = DITHER_SIZE::_SLOW_MODE) {
			this->_size = dither;

			if (dither != DITHER_SIZE::_SLOW_MODE) {
				this->_textures[DITHER_SIZE::_2x2] = std::make_shared<rawrBox::TextureImage>("./content/textures/dither/2x2.png");
				this->_textures[DITHER_SIZE::_3x3] = std::make_shared<rawrBox::TextureImage>("./content/textures/dither/3x3.png");
				this->_textures[DITHER_SIZE::_4x4] = std::make_shared<rawrBox::TextureImage>("./content/textures/dither/4x4.png");
				this->_textures[DITHER_SIZE::_8x8] = std::make_shared<rawrBox::TextureImage>("./content/textures/dither/8x8.png");
			}
		}

		virtual void upload() override {
			bool fastMode = this->_size != DITHER_SIZE::_SLOW_MODE;
			if (fastMode) {
				for (auto t : this->_textures) {
					if (t.second == nullptr) throw std::runtime_error("[RawrBox-Dither] Failed to load texture");
					t.second->upload();
				}
			}

			// Load Shader --------
			bgfx::RendererType::Enum type = bgfx::getRendererType();
			bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, "vs_post_dither");
			bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, "fs_post_dither");

			this->_program = bgfx::createProgram(vsh, fsh, true);
			if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-Dither] Failed to initialize shader program");
			// ------------------

			this->_ditherColor = bgfx::createUniform("s_ditherColor", bgfx::UniformType::Sampler);

			this->_dither_size = bgfx::createUniform("u_dither_size", bgfx::UniformType::Vec4, 1);
			this->_dither_dithering = bgfx::createUniform("u_dithering", bgfx::UniformType::Vec4, 1);
			this->_dithering_intensity = bgfx::createUniform("u_dithering_intensity", bgfx::UniformType::Vec4, 1);
			this->_dithering_depth = bgfx::createUniform("u_dithering_depth", bgfx::UniformType::Vec4, 1);
			this->_dither_color_depth = bgfx::createUniform("u_dithering_color_depth", bgfx::UniformType::Vec4, 1);
			this->_dither_threshold = bgfx::createUniform("u_dithering_threshold", bgfx::UniformType::Vec4, 1);

			rawrBox::UniformUtils::setUniform(this->_dither_size, static_cast<float>(_size));
			rawrBox::UniformUtils::setUniform(this->_dither_dithering, 0.5f);
			rawrBox::UniformUtils::setUniform(this->_dithering_intensity, 10.0f * 0.01f);
			rawrBox::UniformUtils::setUniform(this->_dithering_depth, 5.0f);
			rawrBox::UniformUtils::setUniform(this->_dither_color_depth, std::pow(5.0f, 2));
			rawrBox::UniformUtils::setUniform(this->_dither_threshold, 1.0f);
		}

		virtual void applyEffect() override {
			if (this->_size != DITHER_SIZE::_SLOW_MODE) bgfx::setTexture(1, this->_ditherColor, this->_textures[this->_size]->getHandle());
			bgfx::submit(rawrBox::CURRENT_VIEW_ID, this->_program);
		}
	};
} // namespace rawrBox
