#pragma once
#include <rawrbox/render/postprocess/base.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/texture/image.hpp>

#include <bgfx/bgfx.h>

#include <unordered_map>

namespace rawrbox {
	enum class DITHER_SIZE {
		SLOW_MODE = 0,

		_2x2 = 2,
		_3x3 = 3,
		_4x4 = 4,
		_8x8 = 8,
	};

	class PostProcessPSXDither : public rawrbox::PostProcessBase {
		std::unordered_map<DITHER_SIZE, std::unique_ptr<rawrbox::TextureImage>> _textures = {};

		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _ditherColor = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _dither_size = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _dither_color_depth = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _dithering_intensity = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _dithering_depth = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _dither_threshold = BGFX_INVALID_HANDLE;

	protected:
		DITHER_SIZE _size = DITHER_SIZE::SLOW_MODE;

		// Settings ----
		float _dithering = 0.5F;
		float _intensity = 10.0F;
		float _depth = 5.0F;
		float _colorDepth = 5.0F;
		float _threshold = 1.0F;
		// ---

	public:
		~PostProcessPSXDither() override;

		PostProcessPSXDither(PostProcessPSXDither&&) = delete;
		PostProcessPSXDither& operator=(PostProcessPSXDither&&) = delete;
		PostProcessPSXDither(const PostProcessPSXDither&) = delete;
		PostProcessPSXDither& operator=(const PostProcessPSXDither&) = delete;

		explicit PostProcessPSXDither(DITHER_SIZE dither = DITHER_SIZE::SLOW_MODE);

		virtual void setIntensity(float in);
		virtual void setDepth(float dep);
		virtual void setColorDepth(float dep);
		virtual void setThreshold(float th);

		void upload() override;
		void applyEffect() override;
	};
} // namespace rawrbox
