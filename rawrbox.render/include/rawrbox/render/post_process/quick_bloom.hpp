#pragma once
#include <rawrbox/render/post_process/base.hpp>

namespace rawrbox {
	class PostProcessQuickBloom : public rawrbox::PostProcessBase {
	public:
		explicit PostProcessQuickBloom(float intensity);
		PostProcessQuickBloom(PostProcessQuickBloom&&) = delete;
		PostProcessQuickBloom& operator=(PostProcessQuickBloom&&) = delete;
		PostProcessQuickBloom(const PostProcessQuickBloom&) = delete;
		PostProcessQuickBloom& operator=(const PostProcessQuickBloom&) = delete;
		~PostProcessQuickBloom() override = default;

		virtual void setIntensity(float in);
		void init() override;
	};
} // namespace rawrbox
