#include <rawrbox/render/model/model_mesh.h>

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

	// Generate a simple plane
	void ModelMesh::generatePlane(const rawrBox::Vector3f& pos,const rawrBox::Vector2f& size, const rawrBox::Vector3f& normal, const rawrBox::Color& cl) {
		if(this->_data == nullptr) throw std::runtime_error("No ModelMeshData found");

		// Clear old --
		this->_data->vertices.clear();
		this->_data->indices.clear();
		// ---

		std::array<rawrBox::ModelVertexData, 4> buff;
		buff[0] = {{pos.x, pos.y, pos.z}, {0, 0}, normal, cl};
		buff[2] = {{pos.x, pos.y + size.y, pos.z}, {0, 1}, normal, cl};
		buff[1] = {{pos.x + size.x, pos.y, pos.z}, {1, 0}, normal, cl};
		buff[3] = {{pos.x + size.x, pos.y + size.y, pos.z}, {1, 1}, normal, cl};

		auto& vertices = this->getVertices();
		auto& indices = this->getIndices();

		this->_data->baseVertex = static_cast<uint16_t>(vertices.size());
		this->_data->baseIndex = static_cast<uint16_t>(indices.size());
		this->_data->numVertices += static_cast<uint16_t>(buff.size());
		this->_data->numIndices += 6;

		uint16_t inds[] = {0, 1, 2, 1, 3, 2};
		this->_data->vertices.insert(this->_data->vertices.end(), buff.begin(), buff.end());
		for (uint16_t ind : inds) this->_data->indices.push_back(ind);
	}

	void ModelMesh::generateCube(const rawrBox::Vector3f& pos,const rawrBox::Vector3f& size, const rawrBox::Vector3f& normal, const rawrBox::Color& cl) {
		if(this->_data == nullptr) throw std::runtime_error("No ModelMeshData found");

		// Clear old --
		this->_data->vertices.clear();
		this->_data->indices.clear();
		// ---

		std::array<rawrBox::ModelVertexData, 8> buff;
		buff[0] = {pos + rawrBox::Vector3f(-size.x, size.y, size.z), {0, 0}, normal, rawrBox::Colors::Red};
		buff[1] = {pos + rawrBox::Vector3f(size.x, size.y, size.z), {1, 0}, normal};
		buff[2] = {pos + rawrBox::Vector3f(-size.x, -size.y, size.z), {0, 1}, normal};
		buff[3] = {pos + rawrBox::Vector3f(size.x, -size.y, size.z), {1, 1}, normal, rawrBox::Colors::Purple};

		buff[4] = {pos + rawrBox::Vector3f(-size.x, size.y, -size.z), {0, 0}, normal, rawrBox::Colors::Blue};
		buff[5] = {pos + rawrBox::Vector3f(size.x, size.y, -size.z), {1, 0}, normal};
		buff[6] = {pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), {0, 1}, normal};
		buff[7] = {pos + rawrBox::Vector3f(size.x, -size.y, -size.z), {1, 1}, normal, rawrBox::Colors::Green};

		auto& vertices = this->getVertices();
		auto& indices = this->getIndices();

		this->_data->baseVertex = static_cast<uint16_t>(vertices.size());
		this->_data->baseIndex = static_cast<uint16_t>(indices.size());
		this->_data->numVertices += static_cast<uint16_t>(buff.size());
		this->_data->numIndices += 35;

		uint16_t inds[] = {0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6, 1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7};
		this->_data->vertices.insert(this->_data->vertices.end(), buff.begin(), buff.end());
		for (uint16_t ind : inds) this->_data->indices.push_back(ind);
	}
	// ----
}
