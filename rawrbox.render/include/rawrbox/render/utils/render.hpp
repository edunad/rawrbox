#pragma once

#include <PipelineState.h>
#include <Texture.h>

namespace rawrbox {
	class RenderUtils {
		static Diligent::IPipelineState* _pipe;
		static Diligent::IShaderResourceBinding* _SRB;

		static void init();

	public:
		static void renderQUAD(Diligent::ITextureView* texture, bool transition = true);
	};
} // namespace rawrbox
