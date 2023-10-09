#pragma once
#include <rawrbox/render_temp/postprocess/base.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	class PostProcessStaticNoise : public rawrbox::PostProcessBase {
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _settings = BGFX_INVALID_HANDLE;

		float _strength = 0.1F;

	protected:
	public:
		explicit PostProcessStaticNoise(float s);
		PostProcessStaticNoise(PostProcessStaticNoise&&) = delete;
		PostProcessStaticNoise& operator=(PostProcessStaticNoise&&) = delete;
		PostProcessStaticNoise(const PostProcessStaticNoise&) = delete;
		PostProcessStaticNoise& operator=(const PostProcessStaticNoise&) = delete;
		~PostProcessStaticNoise() override;

		void setStrength(float s);

		void upload() override;
		void applyEffect() override;
	};
} // namespace rawrbox
