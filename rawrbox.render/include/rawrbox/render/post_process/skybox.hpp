#pragma once

#define UINT_DATA
#include <rawrbox/render/post_process/base.hpp>

namespace rawrbox {
	class PostProcessSkybox : public rawrbox::PostProcessBase {
	public:
		explicit PostProcessSkybox(const rawrbox::Colorf& startColor, const rawrbox::Colorf& endColor);
		PostProcessSkybox(PostProcessSkybox&&) = delete;
		PostProcessSkybox& operator=(PostProcessSkybox&&) = delete;
		PostProcessSkybox(const PostProcessSkybox&) = delete;
		PostProcessSkybox& operator=(const PostProcessSkybox&) = delete;
		~PostProcessSkybox() override = default;

		virtual void setStartColor(const rawrbox::Colorf& col);
		virtual void setEndColor(const rawrbox::Colorf& col);
		virtual void setPSXBanding(uint32_t banding);

		void init() override;
	};
} // namespace rawrbox
