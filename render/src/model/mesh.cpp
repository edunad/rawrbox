#include <rawrbox/render/model/mesh.hpp>

namespace rawrbox {
	const std::string& Mesh::getName() const {
		return this->name;
	}

	void Mesh::setName(const std::string& name) {
		this->name = name;
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

	const bool Mesh::empty() const {
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
	void Mesh::setTexture(rawrbox::TextureBase* ptr) { this->texture = ptr; }

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

	void Mesh::setWireframe(bool wireframe) {
		this->wireframe = wireframe;
	}

	void Mesh::setCulling(uint64_t culling) {
		this->culling = culling;
	}

	void Mesh::setDepthTest(uint64_t depthTest) {
		this->depthTest = depthTest;
	}

	void Mesh::setBlend(uint64_t blend) {
		this->blending = blend;
	}

	void Mesh::setRecieveDecals(bool decals) {
		this->addData("mask", {decals ? 1.0F : 0.0F, 0, 0, 0});
	}

	void Mesh::setColor(const rawrbox::Color& color) {
		this->color = color;
	}

	void Mesh::addData(const std::string& id, rawrbox::Vector4f data) { // BGFX shaders only accept vec4, so.. yea
		this->data[id] = data;
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
		for (uint16_t i : other.indices)
			this->indices.push_back(this->totalVertex + i);
		this->vertices.insert(this->vertices.end(), other.vertices.begin(), other.vertices.end());

		this->totalVertex += other.totalVertex;
		this->totalIndex += other.totalIndex;
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

		return this->texture == other.texture &&
		       this->color == other.color &&
		       this->lineMode == other.lineMode &&
		       this->matrix == other.matrix;
	}
} // namespace rawrbox
