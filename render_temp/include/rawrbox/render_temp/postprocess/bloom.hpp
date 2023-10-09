#pragma once
#include <rawrbox/render_temp/postprocess/base.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	class PostProcessBloom : public rawrbox::PostProcessBase {
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _bloom_intensity = BGFX_INVALID_HANDLE;

	protected:
		float _intensity = 0.08F;

	public:
		explicit PostProcessBloom(float intensity);
		PostProcessBloom(PostProcessBloom&&) = delete;
		PostProcessBloom& operator=(PostProcessBloom&&) = delete;
		PostProcessBloom(const PostProcessBloom&) = delete;
		PostProcessBloom& operator=(const PostProcessBloom&) = delete;

		~PostProcessBloom() override;

		virtual void setIntensity(float in);

		void upload() override;
		void applyEffect() override;
	};
} // namespace rawrbox
