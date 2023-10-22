#pragma once
#include <rawrbox/render/post_process/base.hpp>

namespace rawrbox {
	class Test2 : public rawrbox::PostProcessBase {

	public:
		void upload(Diligent::IRenderPass* pass, uint32_t passIndex) override;
		void setRTTexture(Diligent::ITextureView* texture) override;
		void applyEffect() override;
	};
} // namespace rawrbox
