#include <rawrbox/render/models/base.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/utils/pack.hpp>
#include <rawrbox/utils/string.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/render/scripting/wrappers/model/base_wrapper.hpp>
	#include <rawrbox/scripting/scripting.hpp>
#endif

namespace rawrbox {

#ifdef RAWRBOX_SCRIPTING
	void ModelBase::initializeLua() {
		if (!SCRIPTING::initialized) return;
		this->_luaWrapper = sol::make_object(rawrbox::SCRIPTING::getLUA(), rawrbox::ModelBaseWrapper(this->shared_from_this()));
	}
#endif

	void ModelBase::updateBuffers() {
		if (!this->isDynamic() || !this->isUploaded()) return;

		/*const bgfx::Memory* vertMem = bgfx::copy(this->_mesh->vertices.data(), static_cast<uint32_t>(this->_mesh->vertices.size()) * this->_material->vLayout().getStride());
		const bgfx::Memory* indexMem = bgfx::copy(this->_mesh->indices.data(), static_cast<uint32_t>(this->_mesh->indices.size()) * sizeof(uint16_t));

		bgfx::update(this->_vbdh, 0, vertMem);
		bgfx::update(this->_ibdh, 0, indexMem);*/
	}

	ModelBase::~ModelBase() {
		/*RAWRBOX_DESTROY(this->_vbh);
		RAWRBOX_DESTROY(this->_ibh);
		RAWRBOX_DESTROY(this->_vbdh);
		RAWRBOX_DESTROY(this->_ibdh);*/

		this->_mesh.reset();

#ifdef RAWRBOX_SCRIPTING
		if (this->_luaWrapper.valid()) this->_luaWrapper.abandon();
#endif
	}

	// BLEND SHAPES ---
	void ModelBase::applyBlendShapes() {
		/*
				// Reset vertex ---------
				for (auto& shape : this->_blend_shapes) {
					if (!shape.second->isActive() || shape.second->mesh == nullptr) continue;

					auto& verts = shape.second->mesh->vertices;
					for (auto& v : verts)
						v.reset();
				}
				// --------

				for (auto& shape : this->_blend_shapes) {
					if (!shape.second->isActive() || shape.second->mesh == nullptr) continue;

					auto& verts = shape.second->mesh->vertices;

					auto& blendPos = shape.second->pos;
					auto& blendNormals = shape.second->normals;

					if (!blendPos.empty() && blendPos.size() != verts.size()) {
						fmt::print("[RawrBox-ModelBase] Blendshape verts do not match with the mesh '{}' verts! Total verts: {}, blend shape verts: {}", shape.first, verts.size(), blendPos.size());
						return;
					}

					if (!blendNormals.empty() && blendNormals.size() != verts.size()) {
						fmt::print("[RawrBox-ModelBase] Blendshape normals do not match with the mesh '{}' verts! Total verts: {}, blend shape verts: {}", shape.first, verts.size(), blendNormals.size());
						return;
					}

					float step = std::clamp(shape.second->weight, 0.F, 1.F);

					// Apply vertices ----
					for (size_t i = 0; i < blendPos.size(); i++) {
						verts[i].position = verts[i].position.lerp(blendPos[i], step);
					}
					// -------------------

					// Apply normal ----
					for (size_t i = 0; i < blendNormals.size(); i++) {
						auto unmap = rawrbox::Vector4f(rawrbox::PackUtils::fromNormal(verts[i].normal[0])).xyz();
						auto lerp = unmap.lerp(blendNormals[i], step);

						verts[i].normal[0] = rawrbox::PackUtils::packNormal(lerp.x, lerp.y, lerp.z);
					}
					// -------------------
				}*/
	}

	bool ModelBase::removeBlendShape(const std::string& id) {
		auto fnd = this->_blend_shapes.find(id);
		if (fnd == this->_blend_shapes.end()) return false;

		this->_blend_shapes.erase(id);
		this->applyBlendShapes();
		return true;
	}

	bool ModelBase::setBlendShape(const std::string& id, float weight) {
		auto fnd = this->_blend_shapes.find(id);
		if (fnd == this->_blend_shapes.end()) return false;

		fnd->second->weight = weight;
		this->applyBlendShapes();
		return true;
	}

	bool ModelBase::setBlendShapeByKey(const std::string& id, float weight) {
		bool found = false;

		for (auto& s : this->_blend_shapes) {
			auto split = rawrbox::StrUtils::split(s.first, '-');
			auto shapeId = split[split.size() - 1];
			if (shapeId == id) {
				s.second->weight = weight;
				found = true;
			}
		}

		if (found) {
			this->applyBlendShapes();
		}

		return found;
	}
	// --------------

	// UTIL ---
	const rawrbox::Vector3f& ModelBase::getPos() const { return this->_mesh->getPos(); }
	void ModelBase::setPos(const rawrbox::Vector3f& pos) {
		this->_mesh->setPos(pos);
	}

	const rawrbox::Vector3f& ModelBase::getScale() const { return this->_mesh->getScale(); }
	void ModelBase::setScale(const rawrbox::Vector3f& scale) {
		this->_mesh->setScale(scale);
	}

	const rawrbox::Vector4f& ModelBase::getAngle() const { return this->_mesh->getAngle(); }
	void ModelBase::setAngle(const rawrbox::Vector4f& ang) {
		this->_mesh->setAngle(ang);
	}

	void ModelBase::setEulerAngle(const rawrbox::Vector3f& ang) {
		this->_mesh->setEulerAngle(ang);
	}

	const rawrbox::Matrix4x4& ModelBase::getMatrix() const {
		return this->_mesh->matrix;
	}

	bool ModelBase::isDynamic() const {
		return this->_isDynamic;
	}

	bool ModelBase::isUploaded() const {
		return this->_vbh != nullptr && this->_ibh != nullptr;
	}

	rawrbox::Mesh* ModelBase::mesh() {
		return this->_mesh.get();
	}

	// ----
	void ModelBase::upload(bool dynamic) {
		if (this->isUploaded()) throw std::runtime_error("[RawrBox-ModelBase] Upload called twice");

		auto device = rawrbox::RENDERER->device;

		// Generate buffers ----
		this->_isDynamic = dynamic;

		auto vertSize = static_cast<uint32_t>(this->_mesh->vertices.size());
		auto indcSize = static_cast<uint32_t>(this->_mesh->indices.size());

		// VERT ----

		Diligent::BufferDesc VertBuffDesc;
		VertBuffDesc.Name = "RawrBox::Buffer::Vertex";
		VertBuffDesc.Usage = dynamic ? Diligent::USAGE_DYNAMIC : Diligent::USAGE_IMMUTABLE;
		VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
		VertBuffDesc.Size = vertSize * sizeof(this->_mesh->vertices[0]);

		Diligent::BufferData VBData;
		VBData.pData = this->_mesh->vertices.data();
		VBData.DataSize = VertBuffDesc.Size;

		device->CreateBuffer(VertBuffDesc, &VBData, &this->_vbh);
		// ---------------------

		// INDC ----
		Diligent::BufferDesc IndcBuffDesc;
		IndcBuffDesc.Name = "RawrBox::Buffer::Indices";
		IndcBuffDesc.Usage = dynamic ? Diligent::USAGE_DYNAMIC : Diligent::USAGE_IMMUTABLE;
		IndcBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
		IndcBuffDesc.Size = indcSize * sizeof(this->_mesh->indices[0]);

		Diligent::BufferData IBData;
		IBData.pData = this->_mesh->indices.data();
		IBData.DataSize = IndcBuffDesc.Size;

		device->CreateBuffer(IndcBuffDesc, &IBData, &this->_ibh);
		// ---------------------

		this->_material->upload();
	}

	void ModelBase::draw() {
		if (!this->isUploaded()) throw std::runtime_error("[RawrBox-Model] Failed to render model, vertex / index buffer is not uploaded");
	}

#ifdef RAWRBOX_SCRIPTING
	sol::object& ModelBase::getScriptingWrapper() {
		if (!this->_luaWrapper.valid()) this->initializeLua();
		return this->_luaWrapper;
	}
#endif
} // namespace rawrbox
