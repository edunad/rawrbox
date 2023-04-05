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

namespace rawrBox {
	Stencil::Stencil(bgfx::ViewId id, const rawrBox::Vector2& size) {
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
		bgfx::destroy(this->_textureHandle);
		bgfx::destroy(this->_texColor);

		this->_pixelTexture = nullptr;
	}

	void Stencil::initialize() {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, "vs_stencil");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, "fs_stencil");

		this->_2dprogram = bgfx::createProgram(vsh, fsh, true);
		if(this->_2dprogram.idx == 0) throw std::runtime_error("[RawrBOX-Stencil] Failed to initialize shader program");

		this->_pixelTexture = std::make_shared<rawrBox::Texture>(rawrBox::Vector2i(1, 1), Colors::Yellow);
		this->_pixelTexture->upload();

		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
	}

#pragma region RENDERING
	void Stencil::pushVertice(const rawrBox::Vector2& pos, const rawrBox::Vector2& uv, const rawrBox::Color& col) {
		this->_vertices.emplace_back(
			// pos
			pos.x / this->_windowSize.x * 2 - 1,
			(pos.y  / this->_windowSize.y * 2 - 1) * -1,
			0.0f,

			// uv
			static_cast<int16_t>(uv.x),
			static_cast<int16_t>(uv.y),

			// color
			0xFFFFFFFF
			//rawrBox::Color::toBuffer(col)
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
	void Stencil::drawTriangle(const rawrBox::Vector2& a, const rawrBox::Vector2& aUV, const rawrBox::Color& colA, const rawrBox::Vector2& b, const rawrBox::Vector2& bUV, const rawrBox::Color& colB, const rawrBox::Vector2& c, const rawrBox::Vector2& cUV, const rawrBox::Color& colC) {
		if (colA.isTransparent() && colB.isTransparent() && colC.isTransparent()) return;

		this->setTexture(this->_pixelTexture->getHandle());
		this->setShaderProgram(this->_2dprogram);

		/*pushVertice(a + offset, aUV, colA);
		pushVertice(b + offset, bUV, colB);
		pushVertice(c + offset, cUV, colC);*/

		this->pushVertice(a, aUV, colA);
		this->pushVertice(b, bUV, colB);
		this->pushVertice(c, cUV, colC);
		this->pushIndices(3, 2, 1);
	}

	void Stencil::drawBox(const rawrBox::Vector2& pos, const rawrBox::Vector2& size, rawrBox::Color col) {
		this->drawTexture(pos, size, this->_pixelTexture, col);
	}

	void Stencil::drawTexture(rawrBox::Vector2 pos, rawrBox::Vector2 size, std::shared_ptr<rawrBox::Texture> tex, rawrBox::Color col, rawrBox::Vector2 uvStart, rawrBox::Vector2 uvEnd, float rotation, const rawrBox::Vector2& origin) {

		// Texture setup -----
		if(tex == nullptr) throw std::runtime_error("[RawrBOX-Stencil] Invalid texture, cannot draw");

		this->setTexture(tex->getHandle());
		this->setShaderProgram(this->_2dprogram);
		// ------

		// Primitives -----
		rawrBox::Vector2 b = pos;
		rawrBox::Vector2 c = pos;

		b.x += size.x;
		c.y += size.y;

		auto rotOrigin = origin.isNaN() ? pos + size / 2 : pos + origin;

		/*auto vertA = pos.rotateAroundOrigin(rotation, rotOrigin);
		auto vertB = b.rotateAroundOrigin(rotation, rotOrigin);
		auto vertC = c.rotateAroundOrigin(rotation, rotOrigin);
		auto vertD = (pos + size).rotateAroundOrigin(rotation, rotOrigin);

		this->pushVertice(vertA, uvStart, col);
		this->pushVertice(b.rotateAroundOrigin(rotation, rotOrigin), {uvEnd.x, uvStart.y}, col);
		this->pushVertice(c.rotateAroundOrigin(rotation, rotOrigin), {uvStart.x, uvEnd.y}, col);
		this->pushVertice((pos + size).rotateAroundOrigin(rotation, rotOrigin), uvEnd, col);*/

		this->pushVertice(0, 0, col); // 0
		this->pushVertice({100, 0}, {uvEnd.x, uvStart.y}, col); // 1
		this->pushVertice({0, 100}, {uvStart.x, uvEnd.y}, col); // 2
		this->pushVertice({100, 100}, uvEnd, col); // 3


		/*
		0 -- 1

		2 -- 3
		*/

		this->pushIndices(0, 1, 2);
		this->pushIndices(1, 3, 2);
		// ------------------
	}

#pragma endregion



#pragma region RENDERING
	void Stencil::setTexture(const bgfx::TextureHandle& tex) {
		if (this->_textureHandle.idx != tex.idx) draw();
		this->_textureHandle = tex;
	}

	void Stencil::setShaderProgram(const bgfx::ProgramHandle& handle) {
		if (this->_stencilProgram.idx != handle.idx) draw();
		this->_stencilProgram = handle;
	}

	void Stencil::draw() {
		if (this->_vertices.empty() || this->_indices.empty()) return;

		bgfx::setTexture(0, this->_texColor, this->_textureHandle);

        bgfx::setIndexBuffer(this->_ibh);
        bgfx::setVertexBuffer(0, this->_vbh);

		bgfx::setState(0 | BGFX_STATE_DEFAULT);
        bgfx::submit(this->_viewId, this->_stencilProgram);

		this->_vertices.clear();
		this->_indices.clear();
	}
#pragma endregion
}
