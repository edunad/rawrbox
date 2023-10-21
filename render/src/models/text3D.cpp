#include <rawrbox/render/models/text3D.hpp>

#include <utf8.h>

namespace rawrbox {
	uint32_t Text3D::ID = 0;
	Text3D::Text3D() {
		this->setMaterial<rawrbox::MaterialText3D>();
	}

	// UTILS ----
	void Text3D::setScaleMul(float mul) { this->_scaleMul = mul; }
	float Text3D::getScaleMul() const { return this->_scaleMul; }

	uint32_t Text3D::addText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl, rawrbox::Alignment alignX, rawrbox::Alignment alignY, bool billboard) {
		float screenSize = font.getScale() * this->_scaleMul;

		rawrbox::Vector3f startpos = {};
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

		uint32_t id = Text3D::ID++;
		font.render(text, startpos.xy(), true, [this, &font, billboard, pos, startpos, cl, screenSize, id](rawrbox::Glyph* glyph, float x0, float y0, float x1, float y1) {
			rawrbox::Mesh mesh;
			mesh.setPos(pos);

			mesh.setTexture(font.getPackTexture(glyph)); // Set the atlas
			mesh.setOptimizable(!billboard);
			mesh.addData("billboard_mode", {billboard ? 1.F : 0, 0, 0});
			mesh.setName(fmt::format("3dtext-{}", id));

			std::array<rawrbox::VertexNormBoneData, 4> buff{
			    rawrbox::VertexNormBoneData(startpos + Vector3f(x0 * screenSize, y0 * screenSize, 0), rawrbox::Vector2f(glyph->textureTopLeft.x, glyph->textureBottomRight.y), cl),
			    rawrbox::VertexNormBoneData(startpos + Vector3f(x1 * screenSize, y1 * screenSize, 0), rawrbox::Vector2f(glyph->textureBottomRight.x, glyph->textureTopLeft.y), cl),
			    rawrbox::VertexNormBoneData(startpos + Vector3f(x0 * screenSize, y1 * screenSize, 0), rawrbox::Vector2f(glyph->textureTopLeft.x, glyph->textureTopLeft.y), cl),
			    rawrbox::VertexNormBoneData(startpos + Vector3f(x1 * screenSize, y0 * screenSize, 0), rawrbox::Vector2f(glyph->textureBottomRight.x, glyph->textureBottomRight.y), cl),
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

	void Text3D::upload(bool /*dynamic*/) {
		Model::upload(true); // Always force dynamic, since we can remove text
	}

	void Text3D::removeText(uint32_t id) {
		this->removeMeshByName(fmt::format("3dtext-{}", id));
	}
	// ----------
} // namespace rawrbox
