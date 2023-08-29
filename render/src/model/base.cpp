#include <rawrbox/render/model/base.hpp>

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

		const bgfx::Memory* vertMem = bgfx::makeRef(this->_mesh->vertices.data(), static_cast<uint32_t>(this->_mesh->vertices.size()) * this->_material->vLayout().getStride());
		const bgfx::Memory* indexMem = bgfx::makeRef(this->_mesh->indices.data(), static_cast<uint32_t>(this->_mesh->indices.size()) * sizeof(uint16_t));

		bgfx::update(this->_vbdh, 0, vertMem);
		bgfx::update(this->_ibdh, 0, indexMem);
	}

	ModelBase::~ModelBase() {
		RAWRBOX_DESTROY(this->_vbh);
		RAWRBOX_DESTROY(this->_ibh);
		RAWRBOX_DESTROY(this->_vbdh);
		RAWRBOX_DESTROY(this->_ibdh);

		this->_mesh.reset();

#ifdef RAWRBOX_SCRIPTING
		if (this->_luaWrapper.valid()) this->_luaWrapper.abandon();
#endif
	}

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
		if (this->isDynamic()) return bgfx::isValid(this->_ibdh) && bgfx::isValid(this->_vbdh);
		return bgfx::isValid(this->_ibh) && bgfx::isValid(this->_vbh);
	}

	// ----
	void ModelBase::upload(bool dynamic) {
		if (this->isUploaded()) throw std::runtime_error("[RawrBox-ModelBase] Upload called twice");

		// Generate buffers ----
		this->_isDynamic = dynamic;

		const auto layout = this->_material->vLayout();
		const bgfx::Memory* vertMem = bgfx::makeRef(this->_mesh->vertices.data(), static_cast<uint32_t>(this->_mesh->vertices.size()) * layout.getStride());
		const bgfx::Memory* indexMem = bgfx::makeRef(this->_mesh->indices.data(), static_cast<uint32_t>(this->_mesh->indices.size()) * sizeof(uint16_t));

		if (dynamic) {
			if (this->_mesh->empty()) {
				this->_vbdh = bgfx::createDynamicVertexBuffer(1, layout, 0 | BGFX_BUFFER_ALLOW_RESIZE);
				this->_ibdh = bgfx::createDynamicIndexBuffer(1, 0 | BGFX_BUFFER_ALLOW_RESIZE);
			} else {
				this->_vbdh = bgfx::createDynamicVertexBuffer(vertMem, layout, 0 | BGFX_BUFFER_ALLOW_RESIZE);
				this->_ibdh = bgfx::createDynamicIndexBuffer(indexMem, 0 | BGFX_BUFFER_ALLOW_RESIZE);
			}
		} else {
			if (this->_mesh->empty()) throw std::runtime_error("[RawrBox-ModelBase] Static buffer cannot contain empty vertices / indices. Use dynamic buffer instead!");

			this->_vbh = bgfx::createVertexBuffer(vertMem, layout);
			this->_ibh = bgfx::createIndexBuffer(indexMem);
		}
		// -----------------

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