
#include <rawrbox/render/utils/render.hpp>

#include <stdexcept>

namespace rawrbox {
	void RenderUtils::renderScreenQuad(const rawrbox::Vector2i& screenSize) {
		const bgfx::Caps* caps = bgfx::getCaps();
		const bgfx::RendererType::Enum renderer = bgfx::getRendererType();

		float texelHalf = bgfx::RendererType::Direct3D9 == renderer ? 0.5F : 0.0F;

		auto layout = rawrbox::PosUVVertexData::vLayout();
		if (bgfx::getAvailTransientVertexBuffer(3, layout) != 3) return;

		bgfx::TransientVertexBuffer vb = {};
		bgfx::allocTransientVertexBuffer(&vb, 3, layout);
		auto vertex = std::bit_cast<rawrbox::PosUVVertexData*>(vb.data);

		const float minx = -1.F;
		const float maxx = 1.F;
		const float miny = 0.F;
		const float maxy = 1.F * 2.F;

		const float texelHalfW = texelHalf / screenSize.x;
		const float texelHalfH = texelHalf / screenSize.y;
		const float minu = -1.F + texelHalfW;
		const float maxu = 1.F + texelHalfW;

		const float zz = 0.F;

		float minv = texelHalfH;
		float maxv = 2.F + texelHalfH;

		if (caps->originBottomLeft) {
			float temp = minv;
			minv = maxv;
			maxv = temp;

			minv -= 1.F;
			maxv -= 1.F;
		}

		vertex[0] = {{minx, miny, zz}, {minu, minv}};
		vertex[1] = {{maxx, miny, zz}, {maxu, minv}};
		vertex[2] = {{maxx, maxy, zz}, {maxu, maxv}};

		// RENDER -----
		bgfx::setVertexBuffer(0, &vb);
	}

	// NOLINTBEGIN(*)
	void RenderUtils::buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, shaders[1].name);

		program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-GBUFFER] Failed to create shader");
	}

	void RenderUtils::buildComputeShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle csh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);

		program = bgfx::createProgram(csh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-GBUFFER] Failed to create shader");
	}
	// NOLINTEND(*)
} // namespace rawrbox
