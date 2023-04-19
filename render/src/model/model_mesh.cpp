#include <rawrbox/render/model/model_mesh.h>
#include <rawrbox/utils/pack.hpp>

namespace rawrBox {
	ModelMesh::ModelMesh() {
		this->_data = std::make_shared<rawrBox::ModelMeshData>();
	}

	// UTILS ----
	const std::string& ModelMesh::getName() {
		return this->_name;
	}

	void ModelMesh::setName(const std::string& name) {
		this->_name = name;
	}

	std::vector<ModelVertexData>& ModelMesh::getVertices() {
		return this->_data->vertices;
	}

	std::vector<uint16_t>& ModelMesh::getIndices() {
		return this->_data->indices;
	}

	std::shared_ptr<rawrBox::ModelMeshData>& ModelMesh::getData() {
		return this->_data;
	}

	void ModelMesh::setTexture(const std::shared_ptr<rawrBox::TextureBase>& ptr) {
		this->_data->texture = ptr;
	}

	void ModelMesh::setSpecularTexture(const std::shared_ptr<rawrBox::TextureBase>& ptr) {
		this->_data->specular_texture = ptr;
	}

	void ModelMesh::setMatrix(const std::array<float, 16>& offset) {
		this->_data->offsetMatrix = offset;
	}

	void ModelMesh::setWireframe(bool wireframe) {
		this->_data->wireframe = wireframe;
	}

	void ModelMesh::setColor(const rawrBox::Color& color) {
		this->_data->color = color;
	}

	// ------------------------
	void ModelMesh::generatePlane(const rawrBox::Vector3f& pos, const rawrBox::Vector2f& size, const rawrBox::Vector3f& normal, const rawrBox::Color& cl) {
		if (this->_data == nullptr) throw std::runtime_error("No ModelMeshData found");

		// Clear old --
		this->_data->vertices.clear();
		this->_data->indices.clear();
		// ---

		std::array<rawrBox::ModelVertexData, 6> buff = {
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),
		};

		std::array<uint16_t, 6> inds = {
		    0, 1, 2,
		    0, 3, 1};

		this->_data->vertices.insert(this->_data->vertices.end(), buff.begin(), buff.end());
		for (uint16_t ind : inds)
			this->_data->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);
	}

	void ModelMesh::generateCube(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Color& cl) {
		if (this->_data == nullptr) throw std::runtime_error("No ModelMeshData found");

		// Clear old --
		this->_data->vertices.clear();
		this->_data->indices.clear();
		// ---

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

		this->_data->vertices.insert(this->_data->vertices.end(), buff.begin(), buff.end());
		for (uint16_t ind : inds)
			this->_data->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);
	}

	// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
	void ModelMesh::generateGrid(uint32_t size, const rawrBox::Vector3f& pos, const rawrBox::Color& cl) {
		if (this->_data == nullptr) throw std::runtime_error("No ModelMeshData found");

		// Clear old --
		this->_data->vertices.clear();
		this->_data->indices.clear();
		// ---

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

		this->_data->vertices.insert(this->_data->vertices.end(), buff.begin(), buff.end());
		for (uint16_t ind : inds)
			this->_data->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

		this->setWireframe(true);
	}
	// ----
} // namespace rawrBox
