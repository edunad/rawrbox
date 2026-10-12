#pragma once
#include <rawrbox/render/post_process/base.hpp>

namespace rawrbox {
	class PostProcessQuickBloom : public rawrbox::PostProcessBase {
	protected:
		Diligent::IPipelineState* _preFilter = nullptr;
		Diligent::IPipelineState* _blur = nullptr;

		std::unique_ptr<rawrbox::TextureRender> _bloomA;
		std::unique_ptr<rawrbox::TextureRender> _bloomB;

	public:
		explicit PostProcessQuickBloom(float intensity);
		PostProcessQuickBloom(PostProcessQuickBloom&&) = delete;
		PostProcessQuickBloom& operator=(PostProcessQuickBloom&&) = delete;
		PostProcessQuickBloom(const PostProcessQuickBloom&) = delete;
		PostProcessQuickBloom& operator=(const PostProcessQuickBloom&) = delete;
		~PostProcessQuickBloom() override = default;

		virtual void setIntensity(float in);

		void init() override;
		void applyEffect(const rawrbox::TextureBase& source, const rawrbox::TextureRender& target, uint32_t depthID) override;
	};
} // namespace rawrbox