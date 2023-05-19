#pragma once

#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/material/text_unlit.hpp>
#include <rawrbox/render/text/font.hpp>

#include <utf8.h>

#define BGFX_STATE_DEFAULT_3D_TEXT (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA_TO_COVERAGE)
namespace rawrbox {
	class Text3D : public rawrbox::ModelBase<rawrbox::MaterialText3DUnlit> {
	public:
		using ModelBase<rawrbox::MaterialText3DUnlit>::ModelBase;

		// UTILS ----
		void addText(std::weak_ptr<rawrbox::Font> font, const std::string& text, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::White, rawrbox::Alignment alignX = rawrbox::Alignment::Center, rawrbox::Alignment alignY = rawrbox::Alignment::Center, bool billboard = true) {
			if (font.expired()) throw std::runtime_error("[RawrBox-Text3D] Invalid font");

			auto f = font.lock();
			float screenSize = (f->size * 0.025F) / 64.F;

			rawrbox::Vector3f startpos = pos;
			rawrbox::Vector2f tsize = f->getStringSize(text) * screenSize;
			float lineheight = f->getLineHeight();

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

			rawrbox::Vector3 curpos = {0, 0, 0};

			const rawrbox::Glyph* prevGlyph = nullptr;
			while (beginIter != endIter) {
				auto mesh = std::make_shared<rawrbox::Mesh<typename rawrbox::MaterialText3DUnlit::vertexBufferType>>();
				uint32_t point = utf8::next(beginIter, endIter);

				auto& glyph = f->getGlyph(point);
				if (prevGlyph != nullptr) {
					curpos.x += f->getKerning(glyph, *prevGlyph);
				}

				if (point == '\n') {
					curpos.y -= lineheight;
					curpos.x = 0;
					continue;
				}

				rawrbox::Vector3 p = {curpos.x + glyph.bearing.x, curpos.y};
				rawrbox::Vector3 s = {static_cast<float>(glyph.size.x), static_cast<float>(glyph.size.y), 0.F};

				// Set the atlas
				mesh->setTexture(font.lock()->getAtlasTexture(glyph));
				mesh->setCulling(0);
				mesh->setOptimizable(false);
				mesh->addData("billboard_mode", {billboard ? 1.F : 0, 0, 0});
				mesh->vertexPos.translate(pos);

				std::array<typename rawrbox::MaterialText3DUnlit::vertexBufferType, 4> buff{
				    rawrbox::VertexData(startpos + Vector3f(p.x * screenSize, p.y * screenSize, 0), {glyph.textureTopLeft.x, glyph.textureBottomRight.y}, cl),
				    rawrbox::VertexData(startpos + Vector3f((p.x + s.x) * screenSize, (p.y + s.y) * screenSize, 0), {glyph.textureBottomRight.x, glyph.textureTopLeft.y}, cl),
				    rawrbox::VertexData(startpos + Vector3f(p.x * screenSize, (p.y + s.y) * screenSize, 0), {glyph.textureTopLeft.x, glyph.textureTopLeft.y}, cl),
				    rawrbox::VertexData(startpos + Vector3f((p.x + s.x) * screenSize, p.y * screenSize, 0), {glyph.textureBottomRight.x, glyph.textureBottomRight.y}, cl),
				};

				std::array<uint16_t, 6> inds{
				    0, 1, 2,
				    0, 3, 1};

				mesh->baseIndex = mesh->totalIndex;
				mesh->baseVertex = mesh->totalVertex;

				mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
				mesh->indices.insert(mesh->indices.end(), inds.begin(), inds.end());

				mesh->totalVertex += static_cast<uint16_t>(buff.size());
				mesh->totalIndex += static_cast<uint16_t>(inds.size());

				curpos.x += glyph.advance.x;
				curpos.y += glyph.advance.y;

				this->addMesh(std::move(mesh));
			}
		}
		// ----------

		void draw(const rawrbox::Vector3f& camPos) override {
			ModelBase<rawrbox::MaterialText3DUnlit>::draw(camPos);

			for (auto& mesh : this->_meshes) {
				this->_material->process(mesh);
				bgfx::setTransform(this->_matrix.data());

				if (this->isDynamicBuffer()) {
					bgfx::setVertexBuffer(0, this->_vbdh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibdh, mesh->baseIndex, mesh->totalIndex);
				} else {
					bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);
				}

				uint64_t flags = BGFX_STATE_DEFAULT_3D_TEXT | mesh->culling | mesh->blending;
				flags |= mesh->lineMode ? BGFX_STATE_PT_LINES : mesh->wireframe ? BGFX_STATE_PT_LINESTRIP
												: 0;

				bgfx::setState(flags, 0);

				this->_material->postProcess();
			}
		}
	};
} // namespace rawrbox
