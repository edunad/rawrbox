#pragma once

#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/material/text_unlit.hpp>
#include <rawrbox/render/text/font.hpp>

#include <utf8.h>

#define BGFX_STATE_DEFAULT_3D_TEXT (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)
namespace rawrBox {
	class Text3D : public rawrBox::ModelBase<rawrBox::MaterialText3DUnlit> {
	public:
		using ModelBase<rawrBox::MaterialText3DUnlit>::ModelBase;

		bool supportsOptimization() override { return false; }

		// UTILS ----
		void addText(rawrBox::Font* font, const std::string& text, const rawrBox::Vector3f& pos, const rawrBox::Colorf& cl = rawrBox::Colors::White, rawrBox::Alignment alignX = rawrBox::Alignment::Center, rawrBox::Alignment alignY = rawrBox::Alignment::Center, bool billboard = true) {
			float screenSize = (font->size * 0.025F) / 64.F;

			rawrBox::Vector3f startpos = pos;
			rawrBox::Vector2f tsize = font->getStringSize(text) * screenSize;
			float lineheight = font->getLineHeight();

			if (alignX != Alignment::Left || alignY != Alignment::Left) {
				switch (alignX) {
					case Alignment::Left:
						break;
					case Alignment::Center:
						startpos.x -= tsize.x / 2;
						break;
					case Alignment::Right:
						startpos.x -= tsize.x;
						break;
				}

				switch (alignY) {
					case Alignment::Left:
						break;
					case Alignment::Center:
						startpos.y += tsize.y / 2;
						break;
					case Alignment::Right:
						startpos.y += tsize.y;
						break;
				}
			}

			// Glyph rendering
			auto beginIter = text.begin();
			auto endIter = utf8::find_invalid(text.begin(), text.end()); // until invalid

			rawrBox::Vector3 curpos = {0, 0, 0};

			const rawrBox::Glyph* prevGlyph = nullptr;
			while (beginIter != endIter) {
				auto mesh = std::make_shared<rawrBox::Mesh<typename rawrBox::MaterialText3DUnlit::vertexBufferType>>();
				uint32_t point = utf8::next(beginIter, endIter);

				auto& glyph = font->getGlyph(point);
				if (prevGlyph != nullptr) {
					curpos.x += font->getKerning(glyph, *prevGlyph);
				}

				if (point == '\n') {
					curpos.y -= lineheight;
					curpos.x = 0;
					continue;
				}

				rawrBox::Vector3 p = {curpos.x + glyph.bearing.x, curpos.y};
				rawrBox::Vector3 s = {static_cast<float>(glyph.size.x), static_cast<float>(glyph.size.y), 0.F};

				// Set the atlas
				mesh->setTexture(font->getAtlasTexture(glyph));
				mesh->setCulling(0);
				mesh->addData("billboard_mode", {billboard ? 1.F : 0, 0, 0});

				bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

				std::array<typename rawrBox::MaterialText3DUnlit::vertexBufferType, 4> buff{
				    rawrBox::VertexData(startpos + Vector3f(p.x * screenSize, p.y * screenSize, 0), glyph.textureTopLeft.x, glyph.textureBottomRight.y, cl),
				    rawrBox::VertexData(startpos + Vector3f((p.x + s.x) * screenSize, (p.y + s.y) * screenSize, 0), glyph.textureBottomRight.x, glyph.textureTopLeft.y, cl),
				    rawrBox::VertexData(startpos + Vector3f(p.x * screenSize, (p.y + s.y) * screenSize, 0), glyph.textureTopLeft.x, glyph.textureTopLeft.y, cl),
				    rawrBox::VertexData(startpos + Vector3f((p.x + s.x) * screenSize, p.y * screenSize, 0), glyph.textureBottomRight.x, glyph.textureBottomRight.y, cl),
				};

				std::array<uint16_t, 6> inds{
				    0, 1, 2,
				    0, 3, 1};

				mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
				mesh->totalVertex += static_cast<uint16_t>(buff.size());

				for (uint16_t ind : inds)
					mesh->indices.push_back(mesh->totalVertex - ind);
				mesh->totalIndex += static_cast<uint16_t>(inds.size());

				curpos.x += glyph.advance.x;
				curpos.y += glyph.advance.y;

				this->addMesh(std::move(mesh));
			}
		}
		// ----------

		void draw(const rawrBox::Vector3f& camPos) override {
			ModelBase<rawrBox::MaterialText3DUnlit>::draw(camPos);

			for (auto& mesh : this->_meshes) {
				this->_material->process(mesh);

				bgfx::setTransform(this->_matrix.data());

				bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
				bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);

				uint64_t flags = BGFX_STATE_DEFAULT_3D_TEXT | mesh->culling | mesh->blending;
				if (mesh->wireframe) flags |= BGFX_STATE_PT_LINES;

				bgfx::setState(flags, 0);

				this->_material->postProcess();
			}
		}
	};
} // namespace rawrBox
