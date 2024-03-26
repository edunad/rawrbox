#pragma once

#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/text/font.hpp>

namespace rawrbox {

	template <typename M = rawrbox::MaterialText3D>
		requires(std::derived_from<M, rawrbox::MaterialText3D>)
	class Text3D : public rawrbox::Model<M> {
	protected:
		float _scaleMul = 0.45F;

	public:
		Text3D() = default;
		Text3D(const Text3D&) = delete;
		Text3D(Text3D&&) = delete;
		Text3D& operator=(const Text3D&) = delete;
		Text3D& operator=(Text3D&&) = delete;
		~Text3D() override = default;

		// UTILS ----
		void setScaleMul(float mul) { this->_scaleMul = mul; }
		[[nodiscard]] float getScaleMul() const { return this->_scaleMul; }

		size_t addText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::White(), rawrbox::Alignment alignX = rawrbox::Alignment::Center, rawrbox::Alignment alignY = rawrbox::Alignment::Center) {
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

			size_t id = rawrbox::TEXT_ID++;
			font.render(text, startpos.xy(), true, [this, &font, pos, startpos, cl, screenSize, id](rawrbox::Glyph* glyph, float x0, float y0, float x1, float y1) {
				float baselineAdjustment = font.getFontInfo().ascender - glyph->offset.y;
				baselineAdjustment *= screenSize; // Apply screen size scaling to the adjustment

				rawrbox::Mesh<typename M::vertexBufferType> mesh;

				mesh.setTexture(font.getPackTexture(glyph)); // Set the atlas
				mesh.setName(fmt::format("3dtext-{}", id));
				mesh.setColor(cl);

				std::array<rawrbox::VertexData, 4> buff = {
				    rawrbox::VertexData(pos + startpos + Vector3f(x0 * screenSize, (y0 + baselineAdjustment) * screenSize, 0), rawrbox::Vector2f(glyph->textureTopLeft.x, glyph->textureBottomRight.y)),
				    rawrbox::VertexData(pos + startpos + Vector3f(x1 * screenSize, (y1 + baselineAdjustment) * screenSize, 0), rawrbox::Vector2f(glyph->textureBottomRight.x, glyph->textureTopLeft.y)),
				    rawrbox::VertexData(pos + startpos + Vector3f(x0 * screenSize, (y1 + baselineAdjustment) * screenSize, 0), rawrbox::Vector2f(glyph->textureTopLeft.x, glyph->textureTopLeft.y)),
				    rawrbox::VertexData(pos + startpos + Vector3f(x1 * screenSize, (y0 + baselineAdjustment) * screenSize, 0), rawrbox::Vector2f(glyph->textureBottomRight.x, glyph->textureBottomRight.y)),
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

		void removeText(uint32_t indx) {
			this->removeMeshByName(fmt::format("3dtext-{}", indx));
		}
		// ----------

		void upload(rawrbox::UploadType /*type*/ = rawrbox::UploadType::STATIC) override {
			Model<M>::upload(rawrbox::UploadType::RESIZABLE_DYNAMIC); // Always force dynamic, since we can remove text
		}
	};
} // namespace rawrbox
