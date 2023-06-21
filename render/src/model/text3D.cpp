#include <rawrbox/render/model/text3D.hpp>

#include <utf8.h>

#define BGFX_STATE_DEFAULT_3D_TEXT (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)

namespace rawrbox {
	uint32_t Text3D::ID = 0;
	// UTILS ----

	void Text3D::setScaleMul(float mul) { this->_scaleMul = mul; }
	const float Text3D::getScaleMul() const { return this->_scaleMul; }

	uint32_t Text3D::addText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl, rawrbox::Alignment alignX, rawrbox::Alignment alignY, bool billboard) {
		float screenSize = font.getScale() * this->_scaleMul;

		rawrbox::Vector3f startpos = pos;
		rawrbox::Vector2f tsize = font.getStringSize(text) * screenSize;
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
					startpos.z -= tsize.y / 2;
					break;
				case Alignment::Right:
					startpos.z -= tsize.y;
					break;
			}
		}

		uint32_t id = ++Text3D::ID;
		font.render(text, startpos.xy(), true, [this, &font, billboard, pos, startpos, cl, screenSize, id](rawrbox::Glyph* glyph, float x0, float y0, float x1, float y1) {
			rawrbox::Mesh<typename rawrbox::MaterialText3DUnlit::vertexBufferType> mesh;
			mesh.vertexPos.translate(pos);

			mesh.setTexture(font.getPackTexture(glyph)); // Set the atlas
			mesh.setOptimizable(!billboard);
			mesh.addData("billboard_mode", {billboard ? 1.F : 0, 0, 0});
			mesh.setName(fmt::format("3dtext-{}", id));

			mesh.blending = BGFX_STATE_BLEND_ALPHA;
			std::array<typename rawrbox::MaterialText3DUnlit::vertexBufferType, 4> buff{
			    rawrbox::VertexData(startpos + Vector3f(x0 * screenSize, y0 * screenSize, 0), {glyph->textureTopLeft.x, glyph->textureBottomRight.y}, cl),
			    rawrbox::VertexData(startpos + Vector3f(x1 * screenSize, y1 * screenSize, 0), {glyph->textureBottomRight.x, glyph->textureTopLeft.y}, cl),
			    rawrbox::VertexData(startpos + Vector3f(x0 * screenSize, y1 * screenSize, 0), {glyph->textureTopLeft.x, glyph->textureTopLeft.y}, cl),
			    rawrbox::VertexData(startpos + Vector3f(x1 * screenSize, y0 * screenSize, 0), {glyph->textureBottomRight.x, glyph->textureBottomRight.y}, cl),
			};

			std::array<uint16_t, 6> inds{
			    0, 1, 2,
			    0, 3, 1};

			mesh.baseIndex = mesh.totalIndex;
			mesh.baseVertex = mesh.totalVertex;

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

			mesh.totalVertex += static_cast<uint16_t>(buff.size());
			mesh.totalIndex += static_cast<uint16_t>(inds.size());

			this->addMesh(mesh);
		});

		return id;
	}

	void Text3D::upload(bool dynamic) {
		Model<rawrbox::MaterialText3DUnlit>::upload(true); // Always force dynamic, since we can remove text
	}

	void Text3D::removeText(uint32_t id) {
		this->removeMeshByName(fmt::format("3dtext-{}", id));
	}
	// ----------

	void Text3D::draw() {
		ModelBase<rawrbox::MaterialText3DUnlit>::draw();

		for (auto& mesh : this->_meshes) {
			this->_material->process(*mesh);

			bgfx::setTransform((this->getMatrix() * mesh->matrix).data());

			if (this->isDynamicBuffer()) {
				bgfx::setVertexBuffer(0, this->_vbdh, mesh->baseVertex, mesh->totalVertex);
				bgfx::setIndexBuffer(this->_ibdh, mesh->baseIndex, mesh->totalIndex);
			} else {
				bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
				bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);
			}

			uint64_t flags = BGFX_STATE_DEFAULT_3D_TEXT | mesh->culling | mesh->blending | mesh->depthTest;
			if (mesh->lineMode) flags |= BGFX_STATE_PT_LINES;

			bgfx::setState(flags, 0);
			this->_material->postProcess();
			bgfx::discard();
		}
	}
} // namespace rawrbox
