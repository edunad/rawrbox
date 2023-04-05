#pragma once

#include <rawrBOX/render/stencil.h>
#include <rawrBOX/render/texture.h>

#include <bgfx/embedded_shader.h>

// Compiled shaders
#include <generated/shaders/render/all.h>

static const bgfx::EmbeddedShader shaders[] =
{
	BGFX_EMBEDDED_SHADER(vs_stencil),
	BGFX_EMBEDDED_SHADER(fs_stencil),
	BGFX_EMBEDDED_SHADER_END()
};

namespace rawrBOX {
	Stencil::Stencil(bgfx::ViewId id, const rawrBOX::Vector2& size) {
		this->_windowSize = size;
		this->_viewId = id;

		// Shader layout
		this->_vLayout.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();

		// Create buffers
		this->_vbh = bgfx::createVertexBuffer(bgfx::makeRef(this->_vertices.data(), sizeof(this->_vertices)), this->_vLayout);
    	this->_ibh = bgfx::createIndexBuffer(bgfx::makeRef(this->_indices.data(), sizeof(this->_indices)));
	}

	Stencil::~Stencil() {
		// CLEANUP BUFFERS & PROGRAMS
		bgfx::destroy(this->_ibh);
		bgfx::destroy(this->_vbh);
		bgfx::destroy(this->_2dprogram);
	}

	void Stencil::initialize() {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, "vs_stencil");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, "fs_stencil");

		this->_2dprogram = bgfx::createProgram(vsh, fsh, true);
		if(this->_2dprogram.idx == 0) throw std::runtime_error("[RawrBOX-Stencil] Failed to initialize shader program");
	}

#pragma region RENDERING
	void Stencil::pushVertice(const rawrBOX::Vector2& pos, const rawrBOX::Vector2& uv, const rawrBOX::Color& col) {
		this->_vertices.emplace_back(
			// pos
			pos.x / this->_windowSize.x * 2 - 1,
			(pos.y  / this->_windowSize.y * 2 - 1) * -1,
			0.0f,

			// uv
			static_cast<int16_t>(uv.x),
			static_cast<int16_t>(uv.y),

			// color
			0xFFFFFFFF//rawrBOX::Color::toBuffer(col)
		);
	}

	void Stencil::pushIndices(unsigned int a, unsigned int b, unsigned int c) {
		auto pos = static_cast<unsigned int>(this->_vertices.size());

		this->_indices.push_back(pos - a);
		this->_indices.push_back(pos - b);
		this->_indices.push_back(pos - c);
	}
#pragma endregion

#pragma region UTILS
	void Stencil::drawTriangle(const rawrBOX::Vector2& a, const rawrBOX::Vector2& aUV, const rawrBOX::Color& colA, const rawrBOX::Vector2& b, const rawrBOX::Vector2& bUV, const rawrBOX::Color& colB, const rawrBOX::Vector2& c, const rawrBOX::Vector2& cUV, const rawrBOX::Color& colC) {
		if (colA.isTransparent() && colB.isTransparent() && colC.isTransparent()) return;

		/*setTexture(getPixelTexture());
		if(shaderOverride) setShader(shader2D);

		pushVertice(a + offset, aUV, colA);
		pushVertice(b + offset, bUV, colB);
		pushVertice(c + offset, cUV, colC);*/

		pushVertice(a, aUV, colA);
		pushVertice(b, bUV, colB);
		pushVertice(c, cUV, colC);
		pushIndices(3, 2, 1);
	}

	void Stencil::drawBox(const rawrBOX::Vector2& pos, const rawrBOX::Vector2& size) {

	}

	void Stencil::drawTexture(rawrBOX::Vector2 pos, rawrBOX::Vector2 size, std::shared_ptr<rawrBOX::Texture> tex, rawrBOX::Color col, rawrBOX::Vector2 uvStart, rawrBOX::Vector2 uvEnd, float rotation, const rawrBOX::Vector2& origin) {

		// Texture setup -----
		if(tex == nullptr) throw std::runtime_error("[RawrBOX-Stencil] Invalid texture, cannot draw");

		tex->use();
		this->setShaderProgram(this->_2dprogram);
		// ------

		// Primitives -----
		rawrBOX::Vector2 b = pos;
		rawrBOX::Vector2 c = pos;

		b.x += size.x;
		c.y += size.y;

		auto rotOrigin = origin.isNaN() ? pos + size / 2 : pos + origin;

		auto vertA = pos.rotateAroundOrigin(rotation, rotOrigin);
		auto vertB = b.rotateAroundOrigin(rotation, rotOrigin);
		auto vertC = c.rotateAroundOrigin(rotation, rotOrigin);
		auto vertD = (pos + size).rotateAroundOrigin(rotation, rotOrigin);

		pushVertice(vertA, uvStart, col);
		pushVertice(b.rotateAroundOrigin(rotation, rotOrigin), {uvEnd.x, uvStart.y}, col);
		pushVertice(c.rotateAroundOrigin(rotation, rotOrigin), {uvStart.x, uvEnd.y}, col);
		pushVertice((pos + size).rotateAroundOrigin(rotation, rotOrigin), uvEnd, col);

		pushIndices(4, 3, 2);
		pushIndices(3, 2, 1);
		// ------------------
	}

#pragma endregion



#pragma region RENDERING
	void Stencil::setShaderProgram(bgfx::ProgramHandle handle) {
		if (this->_stencilProgram.idx != handle.idx) draw();
		this->_stencilProgram = handle;
	}

	void Stencil::draw() {
		if (this->_vertices.empty() || this->_indices.empty()) return;

        bgfx::setVertexBuffer(0, this->_vbh);
        bgfx::setIndexBuffer(this->_ibh);

		bgfx::setState(this->_viewId | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS);
        bgfx::submit(this->_viewId, this->_stencilProgram);

		this->_vertices.clear();
		this->_indices.clear();
	}
#pragma endregion
}
