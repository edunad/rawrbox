#pragma once
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/texture/flat.h>
#include <rawrbox/utils/pack.hpp>

#include <array>
#include <memory>
#include <vector>

namespace rawrBox {
	struct ModelVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		uint32_t normal;
		uint32_t tangent;
		float u;
		float v;

		uint32_t abgr = 0xFFFFFFFF;

		ModelVertexData() = default;
		ModelVertexData(const rawrBox::Vector3f& _pos, uint32_t _normal, uint32_t _tangent, float _u, float _v, const rawrBox::Color cl = rawrBox::Colors::White) : x(_pos.x), y(_pos.y), z(_pos.z), u(_u), v(_v), normal(_normal), tangent(_tangent), abgr(rawrBox::Color::pack(cl)) {}
		ModelVertexData(float _x, float _y, float _z, uint32_t _normal, uint32_t _tangent, float _u, float _v, uint32_t _abgr) : x(_x), y(_y), z(_z), u(_u), v(_v), normal(_normal), tangent(_tangent), abgr(_abgr) {}
	};

	class Model : public rawrBox::ModelBase<ModelVertexData> {
	protected:
		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _texSpecularColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _offsetColor = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _lightsSettings = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _lightsPosition = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _lightsData = BGFX_INVALID_HANDLE;

		void processLights();

	public:
		~Model();
		Model();

		virtual void upload() override;
		virtual void draw(const rawrBox::Vector3f& camPos) override;

		// UTILS -----
		static std::shared_ptr<rawrBox::Mesh<ModelVertexData>> generatePlane(const rawrBox::Vector3f& pos, const rawrBox::Vector2f& size, const rawrBox::Vector3f& normal = {0, 0, 1}, const rawrBox::Color& cl = rawrBox::Colors::White) {
			std::shared_ptr<rawrBox::Mesh<ModelVertexData>> mesh = std::make_shared<rawrBox::Mesh<ModelVertexData>>();
			auto& data = mesh->getData();

			std::array<rawrBox::ModelVertexData, 6> buff = {
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),
			};

			std::array<uint16_t, 6> inds = {
			    0, 1, 2,
			    0, 3, 1};

			data->vertices.insert(data->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				data->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			return mesh;
		}

		static std::shared_ptr<rawrBox::Mesh<ModelVertexData>> generateCube(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Color& cl = rawrBox::Colors::White) {
			std::shared_ptr<rawrBox::Mesh<ModelVertexData>> mesh = std::make_shared<rawrBox::Mesh<ModelVertexData>>();
			auto& data = mesh->getData();

			std::array<rawrBox::ModelVertexData, 24> buff = {
			    // Back
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 0, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 1, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 1, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 0, 1, cl),

			    // Front
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),

			    // Right
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 1, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 0, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 1, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 0, 0, cl),

			    // Left
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 0, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 1, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 0, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 1, 0, cl),

			    // Top
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),

			    // Bottom
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
			    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl)};

			std::array<uint16_t, 36> inds = {
			    0, 1, 2,
			    0, 3, 1,

			    4, 6, 5,
			    4, 5, 7,

			    8, 9, 10,
			    8, 11, 9,

			    12, 14, 13,
			    12, 13, 15,

			    16, 18, 17,
			    16, 17, 19,

			    20, 21, 22,
			    20, 23, 21};

			data->vertices.insert(data->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				data->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			return mesh;
		}

		// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
		static std::shared_ptr<rawrBox::Mesh<ModelVertexData>> generateGrid(uint32_t size, const rawrBox::Vector3f& pos, const rawrBox::Color& cl = rawrBox::Colors::White) {
			std::shared_ptr<rawrBox::Mesh<ModelVertexData>> mesh = std::make_shared<rawrBox::Mesh<ModelVertexData>>();
			auto& data = mesh->getData();

			std::vector<rawrBox::ModelVertexData> buff = {};
			std::vector<uint16_t> inds = {};

			float step = 1.f;
			for (uint32_t j = 0; j <= size; ++j) {
				for (uint32_t i = 0; i <= size; ++i) {
					float x = static_cast<float>(i) / static_cast<float>(step);
					float y = 0;
					float z = static_cast<float>(j) / static_cast<float>(step);
					auto col = cl;

					if (j == 0 || i == 0 || j >= size || i >= size) col = rawrBox::Colors::DarkGray;
					buff.push_back({rawrBox::Vector3f(pos.x - size / 2, pos.y, pos.z - size / 2) + rawrBox::Vector3f(x, y, z), rawrBox::PackUtils::packNormal(0, 0, 1), rawrBox::PackUtils::packNormal(0, 0, 1), 1, 1, col});
				}
			}

			for (uint32_t j = 0; j < size; ++j) {
				for (uint32_t i = 0; i < size; ++i) {

					uint32_t row1 = j * (size + 1);
					uint32_t row2 = (j + 1) * (size + 1);

					inds.push_back(row1 + i);
					inds.push_back(row1 + i + 1);
					inds.push_back(row1 + i + 1);
					inds.push_back(row2 + i + 1);

					inds.push_back(row2 + i + 1);
					inds.push_back(row2 + i);
					inds.push_back(row2 + i);
					inds.push_back(row1 + i);
				}
			}

			data->vertices.insert(data->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				data->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			data->wireframe = true;
			return mesh;
		}
		// -------
	};
} // namespace rawrBox
