#pragma once

#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/material/text_unlit.hpp>
#include <rawrbox/render/text/font.hpp>

#include <utf8.h>

#define BGFX_STATE_DEFAULT_3D_TEXT (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA)
namespace rawrbox {
	class Text3D : public rawrbox::ModelBase<rawrbox::MaterialText3DUnlit> {
	public:
		using ModelBase<rawrbox::MaterialText3DUnlit>::ModelBase;

		// UTILS ----
		void addText(std::shared_ptr<rawrbox::Font> font, const std::string& text, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::White, rawrbox::Alignment alignX = rawrbox::Alignment::Center, rawrbox::Alignment alignY = rawrbox::Alignment::Center, bool billboard = true) {
			if (font == nullptr) throw std::runtime_error("[RawrBox-Text3D] Invalid font");

			/*
						float screenSize = (f->getSize() * 0.025F) / 64.F;

						rawrbox::Vector3f startpos = pos;
						rawrbox::Vector2f tsize = f->getStringSize(text) * screenSize;

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
									startpos.y -= tsize.y / 2;
									break;
								case Alignment::Right:
									startpos.y -= tsize.y;
									break;
							}
						}

						startpos.x = std::roundf(startpos.x);
						startpos.y = std::roundf(startpos.y);

						auto info = f->getFontInfo();
						const float lineHeight = f->getLineHeight();
						startpos.y += lineHeight + info.descender;

						rawrbox::Vector3f cursor = startpos;
						uint16_t prevCodePoint = 0;

						auto beginIter = text.begin();
						auto endIter = utf8::find_invalid(text.begin(), text.end()); // Find invalid utf8

						while (beginIter != endIter) {
							uint16_t point = utf8::next(beginIter, endIter); // get codepoint
							if (point == L'\n') {
								cursor.y -= lineHeight;
								cursor.x = startpos.x;

								prevCodePoint = 0;
								continue;
							}

							const auto glyph = f->getGlyph(point);
							if (glyph.expired()) continue;

							auto gl = glyph.lock();
							float kerning = f->getKerning(prevCodePoint, point);
							cursor.x += kerning;

							float x0 = (cursor.x + (gl->offset.x)) * screenSize;
							float y0 = (cursor.y + (gl->offset.y)) * screenSize;
							float x1 = (x0 + gl->size.x) * screenSize;
							float y1 = (y0 + gl->size.y) * screenSize;

							auto mesh = std::make_shared<rawrbox::Mesh<typename rawrbox::MaterialText3DUnlit::vertexBufferType>>();
							// Set the atlas
							mesh->setTexture(font.lock()->getAtlasTexture(glyph));
							mesh->setCulling(0);
							mesh->setOptimizable(false);
							mesh->addData("billboard_mode", {billboard ? 1.F : 0, 0, 0});
							mesh->vertexPos.translate(pos);

							std::array<typename rawrbox::MaterialText3DUnlit::vertexBufferType, 4> buff{
							    rawrbox::VertexData(startpos + Vector3f(x0, y0, 0), {gl->textureTopLeft.x, gl->textureBottomRight.y}, cl),
							    rawrbox::VertexData(startpos + Vector3f(x0, y1, 0), {gl->textureBottomRight.x, gl->textureTopLeft.y}, cl),
							    rawrbox::VertexData(startpos + Vector3f(x1, y0, 0), {gl->textureTopLeft.x, gl->textureTopLeft.y}, cl),
							    rawrbox::VertexData(startpos + Vector3f(x1, y1, 0), {gl->textureBottomRight.x, gl->textureBottomRight.y}, cl),
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

							cursor.x += gl->advance.x;
							this->addMesh(std::move(mesh));
						}*/
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
