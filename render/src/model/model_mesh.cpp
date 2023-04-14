#include <rawrbox/render/model/model_mesh.h>
#include <rawrbox/utils/pack.hpp>

namespace rawrBox {
	ModelMesh::ModelMesh(const std::string& name_) {
		this->_name = name_;
		this->_data = std::make_shared<rawrBox::ModelMeshData>();
	}

	// UTILS ----
	const std::string& ModelMesh::getName() {
		return this->_name;
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

	void ModelMesh::setOffset(const std::array<float, 16>& offset) {
		this->_data->offsetMatrix = offset;
	}

	// ------------------------
	void ModelMesh::generatePlane(const rawrBox::Vector3f& pos, const rawrBox::Vector2f& size, const rawrBox::Vector3f& normal, const rawrBox::Color& cl) {
		if (this->_data == nullptr) throw std::runtime_error("No ModelMeshData found");

		// Clear old --
		this->_data->vertices.clear();
		this->_data->indices.clear();
		// ---

		std::array<rawrBox::ModelVertexData, 6> buff = {
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0x7fff, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0x7fff, 0x7fff, cl),
		};

		std::array<uint16_t, 6> inds = {
		    0, 1, 2,
		    0, 3, 1};

		this->_data->numVertices = static_cast<uint16_t>(buff.size());
		this->_data->numIndices = static_cast<uint16_t>(inds.size());

		this->_data->vertices.insert(this->_data->vertices.end(), buff.begin(), buff.end());
		for (uint16_t ind : inds)
			this->_data->indices.push_back(this->_data->numVertices - ind);
	}

	void ModelMesh::generateCube(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Color& cl) {
		if (this->_data == nullptr) throw std::runtime_error("No ModelMeshData found");

		// Clear old --
		this->_data->vertices.clear();
		this->_data->indices.clear();
		// ---

		std::array<rawrBox::ModelVertexData, 24> buff = {
		    // Back
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0x7fff, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0x7fff, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 0x7fff, cl),
		    // Front
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0x7fff, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0x7fff, 0x7fff, cl),
		    // Right
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0x7fff, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0x7fff, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 0, cl),
		    // Left
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0x7fff, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0x7fff, 0, cl),
		    // Top
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0x7fff, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0x7fff, 0x7fff, cl),
		    // Bottom
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0x7fff, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0x7fff, cl),
		    ModelVertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0x7fff, 0, cl)};

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

		this->_data->numVertices = static_cast<uint16_t>(buff.size());
		this->_data->numIndices = static_cast<uint16_t>(inds.size());

		this->_data->vertices.insert(this->_data->vertices.end(), buff.begin(), buff.end());
		for (uint16_t ind : inds)
			this->_data->indices.push_back(this->_data->numVertices - ind);
	}
	// ----
} // namespace rawrBox
