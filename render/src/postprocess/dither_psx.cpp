
#include <rawrbox/render/postprocess/dither_psx.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>
#include <rawrbox/render/utils/uniforms.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader dither_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_post_base),
    BGFX_EMBEDDED_SHADER(fs_post_dither),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	PostProcessPSXDither::~PostProcessPSXDither() {
		this->_textures.clear();

		RAWRBOX_DESTROY(this->_program);

		RAWRBOX_DESTROY(this->_ditherColor);

		RAWRBOX_DESTROY(this->_dither_size);
		RAWRBOX_DESTROY(this->_dither_color_depth);
		RAWRBOX_DESTROY(this->_dithering_intensity);
		RAWRBOX_DESTROY(this->_dithering_depth);
		RAWRBOX_DESTROY(this->_dither_threshold);
	}

	PostProcessPSXDither::PostProcessPSXDither(rawrbox::DITHER_SIZE dither) : _size(dither) {
		if (dither != DITHER_SIZE::SLOW_MODE) {
			this->_textures[DITHER_SIZE::_2x2] = std::make_unique<rawrbox::TextureImage>("./content/textures/dither/2x2.png");
			this->_textures[DITHER_SIZE::_3x3] = std::make_unique<rawrbox::TextureImage>("./content/textures/dither/3x3.png");
			this->_textures[DITHER_SIZE::_4x4] = std::make_unique<rawrbox::TextureImage>("./content/textures/dither/4x4.png");
			this->_textures[DITHER_SIZE::_8x8] = std::make_unique<rawrbox::TextureImage>("./content/textures/dither/8x8.png");
		}
	}

	void PostProcessPSXDither::setIntensity(float in) {
		this->_intensity = in;
		if (bgfx::isValid(this->_dithering_intensity)) rawrbox::UniformUtils::setUniform(this->_dithering_intensity, this->_intensity * 0.01F);
	}

	void PostProcessPSXDither::setDepth(float dep) {
		this->_depth = dep;
		if (bgfx::isValid(this->_dithering_depth)) rawrbox::UniformUtils::setUniform(this->_dithering_depth, this->_depth);
	}

	void PostProcessPSXDither::setColorDepth(float dep) {
		this->_colorDepth = dep;
		if (bgfx::isValid(this->_dither_color_depth)) rawrbox::UniformUtils::setUniform(this->_dither_color_depth, std::pow(this->_colorDepth, 2));
	}

	void PostProcessPSXDither::setThreshold(float th) {
		this->_threshold = th;
		if (bgfx::isValid(this->_dither_threshold)) rawrbox::UniformUtils::setUniform(this->_dither_threshold, this->_threshold);
	}

	void PostProcessPSXDither::upload() {
		bool fastMode = this->_size != DITHER_SIZE::SLOW_MODE;

		if (fastMode) {
			for (auto& t : this->_textures) {
				if (t.second == nullptr) throw std::runtime_error("[RawrBox-Dither] Failed to load texture");
				t.second->upload();
			}
		}

		// Load Shader --------
		rawrbox::RenderUtils::buildShader(dither_shaders, this->_program);
		// ------------------

		this->_ditherColor = bgfx::createUniform("s_ditherColor", bgfx::UniformType::Sampler);
		this->_dither_size = bgfx::createUniform("u_dither_size", bgfx::UniformType::Vec4, 1);

		this->_dithering_intensity = bgfx::createUniform("u_dithering_intensity", bgfx::UniformType::Vec4, 1);
		this->_dithering_depth = bgfx::createUniform("u_dithering_depth", bgfx::UniformType::Vec4, 1);
		this->_dither_color_depth = bgfx::createUniform("u_dithering_color_depth", bgfx::UniformType::Vec4, 1);
		this->_dither_threshold = bgfx::createUniform("u_dithering_threshold", bgfx::UniformType::Vec4, 1);

		rawrbox::UniformUtils::setUniform(this->_dither_size, static_cast<float>(_size));

		rawrbox::UniformUtils::setUniform(this->_dithering_intensity, this->_intensity * 0.01F);
		rawrbox::UniformUtils::setUniform(this->_dithering_depth, this->_depth);
		rawrbox::UniformUtils::setUniform(this->_dither_color_depth, std::pow(this->_colorDepth, 2));
		rawrbox::UniformUtils::setUniform(this->_dither_threshold, this->_threshold);
	}

	void PostProcessPSXDither::applyEffect() {
		if (this->_size != DITHER_SIZE::SLOW_MODE) {
			bgfx::setTexture(1, this->_ditherColor, this->_textures[this->_size]->getHandle());
		} else {
			bgfx::setTexture(1, this->_ditherColor, rawrbox::WHITE_TEXTURE->getHandle());
		}

		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
	}
} // namespace rawrbox
