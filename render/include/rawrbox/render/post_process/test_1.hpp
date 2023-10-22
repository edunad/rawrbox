#pragma once
#include <rawrbox/render/post_process/base.hpp>

#include "Common/interface/RefCntAutoPtr.hpp"

namespace rawrbox {
	class Test1 : public rawrbox::PostProcessBase {
		Diligent::RefCntAutoPtr<Diligent::IPipelineState> test;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> testsrg;

	public:
		void upload(Diligent::IRenderPass* pass, uint32_t passIndex) override;
		void setRTTexture(Diligent::ITextureView* texture) override;
		void applyEffect() override;
	};
} // namespace rawrbox
