#pragma once
#include <rawrbox/render/model/base.hpp>

#include <cstdint>

namespace rawrBox {
	struct SpriteVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		float u;
		float v;

		uint32_t abgr = 0xFFFFFFFF;

		SpriteVertexData() = default;
		SpriteVertexData(const rawrBox::Vector3f& _pos, float _u, float _v, const rawrBox::Color cl = rawrBox::Colors::White) : x(_pos.x), y(_pos.y), z(_pos.z), u(_u), v(_v), abgr(rawrBox::Color::pack(cl)) {}
		SpriteVertexData(float _x, float _y, float _z, float _u, float _v, uint32_t _abgr) : x(_x), y(_y), z(_z), u(_u), v(_v), abgr(_abgr) {}
	};

	class Sprite : public rawrBox::ModelBase<SpriteVertexData> {
		bgfx::UniformHandle _spritePos = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _offsetColor = BGFX_INVALID_HANDLE;

	public:
		Sprite();
		~Sprite();

		virtual void upload() override;
		virtual void draw(const rawrBox::Vector3f& camPos) override;

		// UTILS -----
		static std::shared_ptr<rawrBox::Mesh<SpriteVertexData>> generatePlane(const rawrBox::Vector3f& pos, const rawrBox::Vector2f& size, const rawrBox::Vector3f& normal = {0, 0, 1}, const rawrBox::Color& cl = rawrBox::Colors::White) {
			std::shared_ptr<rawrBox::Mesh<SpriteVertexData>> mesh = std::make_shared<rawrBox::Mesh<SpriteVertexData>>();

			auto& data = mesh->getData();
			bx::mtxTranslate(data->offsetMatrix.data(), pos.x, pos.y, pos.z);

			std::array<rawrBox::SpriteVertexData, 6> buff = {
			    SpriteVertexData(rawrBox::Vector3f(-size.x, -size.y, 0), 0, 1, cl),
			    SpriteVertexData(rawrBox::Vector3f(size.x, size.y, 0), 1, 0, cl),
			    SpriteVertexData(rawrBox::Vector3f(-size.x, size.y, 0), 0, 0, cl),
			    SpriteVertexData(rawrBox::Vector3f(size.x, -size.y, 0), 1, 1, cl),
			};

			std::array<uint16_t, 6> inds = {
			    0, 1, 2,
			    0, 3, 1};

			data->totalVertex = static_cast<uint16_t>(buff.size());
			data->totalIndex = static_cast<uint16_t>(inds.size());

			data->vertices.insert(data->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				data->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			return mesh;
		}
	};
} // namespace rawrBox
