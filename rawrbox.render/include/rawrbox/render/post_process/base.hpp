#pragma once

#include <PipelineState.h>
#include <ShaderResourceBinding.h>
#include <TextureView.h>

namespace rawrbox {
	class PostProcessBase {
	protected:
		Diligent::IPipelineState* _pipeline = nullptr;
		Diligent::IShaderResourceBinding* _bind = nullptr;

	public:
		PostProcessBase() = default;
		PostProcessBase(PostProcessBase&&) = delete;
		PostProcessBase& operator=(PostProcessBase&&) = delete;
		PostProcessBase(const PostProcessBase&) = delete;
		PostProcessBase& operator=(const PostProcessBase&) = delete;

		virtual ~PostProcessBase() = default;

		virtual void upload() = 0;
		virtual void applyEffect(Diligent::ITextureView* texture) = 0;
	};
} // namespace rawrbox
