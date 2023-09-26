#include <rawrbox/render/model/mesh.hpp>

namespace rawrbox {
	const std::string& Mesh::getName() const {
		return this->name;
	}

	void Mesh::setName(const std::string& _name) {
		this->name = _name;
	}

	const std::vector<rawrbox::VertexData>& Mesh::getVertices() const {
		return this->vertices;
	}

	const std::vector<uint16_t>& Mesh::getIndices() const {
		return this->indices;
	}

	const rawrbox::BBOX& Mesh::getBBOX() const {
		return this->bbox;
	}

	bool Mesh::empty() const {
		return this->indices.empty() || this->vertices.empty();
	}

	const rawrbox::Matrix4x4& Mesh::getMatrix() { return this->matrix; }

	const rawrbox::Vector3f& Mesh::getPos() const { return this->_pos; }
	void Mesh::setPos(const rawrbox::Vector3f& pos) {
		this->_pos = pos;
		this->matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
	}

	const rawrbox::Vector4f& Mesh::getAngle() const { return this->_angle; }
	void Mesh::setAngle(const rawrbox::Vector4f& ang) {
		this->_angle = ang;
		this->matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
	}

	void Mesh::setEulerAngle(const rawrbox::Vector3f& ang) {
		this->_angle = rawrbox::Vector4f::toQuat(ang);
		this->matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
	}

	const rawrbox::Vector3f& Mesh::getScale() const { return this->_scale; }
	void Mesh::setScale(const rawrbox::Vector3f& scale) {
		this->_scale = scale;
		this->matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
	}

	const rawrbox::TextureBase* Mesh::getTexture() const { return this->texture; }
	void Mesh::setTexture(rawrbox::TextureBase* ptr) {
		this->texture = ptr;
	}

	uint16_t Mesh::getAtlasID(int index) const {
		if (this->vertices.empty()) return 0;
		if (index < 0) return static_cast<uint16_t>(this->vertices.front().uv.z);

		return static_cast<uint16_t>(this->vertices[std::clamp(index, 0, static_cast<int>(this->vertices.size() - 1))].uv.z);
	}

	void Mesh::setAtlasID(uint16_t _atlasId, int index) {
		auto vSize = static_cast<int>(this->vertices.size());
		for (int i = 0; i < vSize; i++) {
			if (index != -1 && i == index) {
				this->vertices[i].setAtlasId(_atlasId);
				break;
			}

			this->vertices[i].setAtlasId(_atlasId);
		}
	}

	const rawrbox::TextureBase* Mesh::getNormalTexture() const { return this->normalTexture; }
	void Mesh::setNormalTexture(rawrbox::TextureBase* ptr) { this->normalTexture = ptr; }

	const rawrbox::TextureBase* Mesh::getDisplacementTexture() const { return this->displacementTexture; }
	void Mesh::setDisplacementTexture(rawrbox::TextureBase* ptr, float power) {
		this->displacementTexture = ptr;
		this->addData("displacement_strength", {power, 0, 0, 0});
		this->setOptimizable(false);
	}

	const rawrbox::TextureBase* Mesh::getEmissionTexture() const { return this->emissionTexture; }
	void Mesh::setEmissionTexture(rawrbox::TextureBase* ptr, float intensity) {
		this->emissionTexture = ptr;
		this->emissionIntensity = intensity;
	}

	const rawrbox::TextureBase* Mesh::getSpecularTexture() const { return this->specularTexture; }
	void Mesh::setSpecularTexture(rawrbox::TextureBase* ptr, float shininess) {
		this->specularTexture = ptr;
		this->specularShininess = shininess;
	}

	void Mesh::setVertexSnap(float power) {
		this->addData("vertex_snap", {power, 0, 0, 0});
		this->setOptimizable(false);
	}

	void Mesh::setWireframe(bool _wireframe) {
		this->wireframe = _wireframe;
	}

	void Mesh::setCulling(uint64_t _culling) {
		this->culling = _culling;
	}

	void Mesh::setDepthTest(uint64_t _depthTest) {
		this->depthTest = _depthTest;
	}

	void Mesh::setBlend(uint64_t blend) {
		this->blending = blend;
	}

	void Mesh::setRecieveDecals(bool decals) {
		this->addData("mask", {decals ? 1.0F : 0.0F, 0, 0, 0});
	}

	uint32_t Mesh::getId(int index) const {
		if (this->vertices.empty()) return 0;
		if (index < 0) return this->vertices.front().id;

		return this->vertices[std::clamp(index, 0, static_cast<int>(this->vertices.size() - 1))].id;
	}

	void Mesh::setId(uint32_t id, int index) {
		auto vSize = static_cast<int>(this->vertices.size());
		for (int i = 0; i < vSize; i++) {
			if (index != -1 && i == index) {
				this->vertices[i].setId(id);
				break;
			}

			this->vertices[i].setId(id);
		}
	}

	void Mesh::setColor(const rawrbox::Color& _color) {
		this->color = _color;
	}

	void Mesh::addData(const std::string& id, rawrbox::Vector4f _data) { // BGFX shaders only accept vec4, so.. yea
		this->data[id] = _data;
	}

	const rawrbox::Vector4f& Mesh::getData(const std::string& id) const {
		auto fnd = this->data.find(id);
		if (fnd == this->data.end()) throw std::runtime_error(fmt::format("[RawrBox-Mesh] Data '{}' not found", id));
		return fnd->second;
	}

	bool Mesh::hasData(const std::string& id) const {
		return this->data.find(id) != this->data.end();
	}

	rawrbox::Skeleton* Mesh::getSkeleton() const {
		return this->skeleton;
	}

	void Mesh::merge(const rawrbox::Mesh& other) {
		rawrbox::Vector3f offset = (other._pos != this->_pos) ? other._pos - this->_pos : rawrbox::Vector3f(0, 0, 0);

		for (uint16_t i : other.indices) {
			this->indices.push_back(this->totalVertex + i);
		}

		if (offset == rawrbox::Vector3f::zero()) {
			this->vertices.insert(this->vertices.end(), other.vertices.begin(), other.vertices.end());
		} else {
			for (auto v : other.vertices) {
				v.position += offset;
				this->vertices.push_back(v);
			}
		}

		this->totalVertex += other.totalVertex;
		this->totalIndex += other.totalIndex;
	}

	void Mesh::rotateVertices(float rad, rawrbox::Vector3f axis) {
		for (auto& v : vertices) {
			v.position = v.position.rotateAroundOrigin(axis, rad);
		}
	}

	void Mesh::clear() {
		this->vertices.clear();
		this->indices.clear();

		this->totalIndex = 0;
		this->totalVertex = 0;
		this->baseIndex = 0;
		this->baseVertex = 0;
	}

	void Mesh::setOptimizable(bool status) { this->_canOptimize = status; }
	bool Mesh::canOptimize(const rawrbox::Mesh& other) const {
		if (!this->_canOptimize || !other._canOptimize) return false;
		if (this->vertices.size() + other.vertices.size() >= 16000) return false; // Max vertice limit
		if (this->indices.size() + other.indices.size() >= 16000) return false;   // Max indice limit

		return this->texture == other.texture &&
		       this->color == other.color &&
		       this->lineMode == other.lineMode &&
		       this->matrix == other.matrix;
	}
} // namespace rawrbox
