
#include <rawrbox/render/utils/render.hpp>

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

		bgfx::setVertexBuffer(0, &vb);
	}
} // namespace rawrbox
