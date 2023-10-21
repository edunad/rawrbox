#pragma once

#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <Graphics/GraphicsEngine/interface/Texture.h>

namespace rawrbox {
	class RenderUtils {
		static Diligent::IPipelineState* _pipe;
		static Diligent::IShaderResourceBinding* _SRB;

		static void init();

	public:
		static void renderQUAD(Diligent::ITextureView* texture);
	};
} // namespace rawrbox
