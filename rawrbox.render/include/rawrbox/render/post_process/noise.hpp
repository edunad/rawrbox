#pragma once

#include <rawrbox/render/post_process/base.hpp>

namespace rawrbox {

	class PostProcessNoise : public rawrbox::PostProcessBase {
	public:
		explicit PostProcessNoise(float intensity);
		PostProcessNoise(PostProcessNoise&&) = delete;
		PostProcessNoise& operator=(PostProcessNoise&&) = delete;
		PostProcessNoise(const PostProcessNoise&) = delete;
		PostProcessNoise& operator=(const PostProcessNoise&) = delete;
		~PostProcessNoise() override = default;

		virtual void setIntensity(float in);

		void init() override;
		void applyEffect(const rawrbox::TextureBase& texture) override;
	};
} // namespace rawrbox
