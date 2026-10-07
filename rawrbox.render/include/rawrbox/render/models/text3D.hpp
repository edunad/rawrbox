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
			const float screenSize = font.getScale() * this->_scaleMul;

			const rawrbox::Vector2f tsize = font.getStringSize(text);
			rawrbox::Vector2f offset = {};

			switch (alignX) {
				case Alignment::Left:
					break;
				case Alignment::Center:
					offset.x -= tsize.x / 2;
					break;
				case Alignment::Right:
					offset.x -= tsize.x;
					break;
			}

			switch (alignY) {
				case Alignment::Left: // Top
					break;
				case Alignment::Center:
					offset.y += tsize.y / 2;
					break;
				case Alignment::Right: // Bottom
					offset.y += tsize.y;
					break;
			}

			size_t id = rawrbox::TEXT_ID++;

			font.render(text, {}, false, [this, &font, pos, offset, cl, screenSize, id](rawrbox::Glyph* glyph, float x0, float y0, float x1, float y1) {
				const float left = (offset.x + x0) * screenSize;
				const float right = (offset.x + x1) * screenSize;
				const float top = (offset.y - y0) * screenSize;
				const float bottom = (offset.y - y1) * screenSize;

				rawrbox::Mesh<typename M::vertexBufferType> mesh;

				mesh.setTexture(font.getPackTexture(glyph)); // Set the atlas
				mesh.setName(fmt::format("3dtext-{}", id));
				mesh.setColor(cl);

				std::array<rawrbox::VertexUVData, 4> buff = {
				    rawrbox::VertexUVData(pos + Vector3f(left, bottom, 0), rawrbox::Vector2f(glyph->textureTopLeft.x, glyph->textureBottomRight.y)),
				    rawrbox::VertexUVData(pos + Vector3f(right, top, 0), rawrbox::Vector2f(glyph->textureBottomRight.x, glyph->textureTopLeft.y)),
				    rawrbox::VertexUVData(pos + Vector3f(left, top, 0), rawrbox::Vector2f(glyph->textureTopLeft.x, glyph->textureTopLeft.y)),
				    rawrbox::VertexUVData(pos + Vector3f(right, bottom, 0), rawrbox::Vector2f(glyph->textureBottomRight.x, glyph->textureBottomRight.y)),
				};

				std::array<uint32_t, 6> inds{
				    0, 1, 2,
				    0, 3, 1};

				mesh.baseIndex = mesh.totalIndex;
				mesh.baseVertex = mesh.totalVertex;

				mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
				mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

				mesh.totalVertex += static_cast<uint32_t>(buff.size());
				mesh.totalIndex += static_cast<uint32_t>(inds.size());

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
