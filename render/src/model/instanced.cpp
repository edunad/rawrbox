
#include <rawrbox/render/model/instanced.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/scripting/scripting.hpp>
#endif

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)

namespace rawrbox {
	void InstancedModel::updateBuffers() {
		rawrbox::ModelBase::updateBuffers();
		this->updateInstance();
	}

#ifdef RAWRBOX_SCRIPTING
	void InstancedModel::initializeLua() {
		if (this->_luaWrapper.valid()) this->_luaWrapper.abandon();
		this->_luaWrapper = sol::make_object(rawrbox::SCRIPTING::getLUA(), rawrbox::InstancedModelWrapper(this->shared_from_this()));
	}
#endif

	InstancedModel::InstancedModel(size_t instanceSize) {
		if (instanceSize != 0) this->_instances.reserve(instanceSize);
		this->setMaterial<rawrbox::MaterialInstanced>();
	}

	InstancedModel::~InstancedModel() {
		RAWRBOX_DESTROY(this->_dataBuffer);
		this->_instances.clear();
	}

	void InstancedModel::setAutoUpload(bool enabled) {
		this->_autoUpload = enabled;
	}

	void InstancedModel::setTemplate(rawrbox::Mesh mesh) {
		if (mesh.empty()) throw std::runtime_error("[RawrBox-InstancedModel] Invalid mesh! Missing vertices / indices!");
		this->_mesh = std::make_unique<rawrbox::Mesh>(mesh);

		if (this->isUploaded() && this->isDynamic()) {
			this->updateBuffers();
		}
	}

	rawrbox::Mesh& InstancedModel::getTemplate() const {
		if (this->_mesh == nullptr) throw std::runtime_error("[RawrBox-InstancedModel] Invalid mesh! Missing vertices / indices!");
		return *this->_mesh;
	}

	void InstancedModel::addInstance(const rawrbox::Instance& instance) {
		this->_instances.push_back(instance);
		if (this->isUploaded() && this->_autoUpload) this->updateInstance();
	}

	void InstancedModel::removeInstance(size_t i) {
		if (i < 0 || i >= this->_instances.size()) throw std::runtime_error("[RawrBox-InstancedModel] Failed to find instance");
		this->_instances.erase(this->_instances.begin() + i);

		if (this->isUploaded() && this->_autoUpload) this->updateInstance();
	}

	[[nodiscard]] rawrbox::Instance& InstancedModel::getInstance(size_t i) {
		if (i < 0 || i >= this->_instances.size()) throw std::runtime_error("[RawrBox-InstancedModel] Failed to find instance");
		return this->_instances[i];
	}

	std::vector<rawrbox::Instance>& InstancedModel::instances() { return this->_instances; }
	size_t InstancedModel::count() const { return this->_instances.size(); }

	void InstancedModel::upload(bool /*dynamic*/) {
		rawrbox::ModelBase::upload(false);

		this->_dataBuffer = bgfx::createDynamicVertexBuffer(
		    1, rawrbox::Instance::vLayout(), BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);

		this->updateInstance();
	}

	void InstancedModel::updateInstance() {
		if (this->_instances.empty()) return;
		if (!bgfx::isValid(this->_dataBuffer)) throw std::runtime_error("[RawrBox-InstancedModel] Data buffer not valid! Did you call upload()?");

		const bgfx::Memory* mem = bgfx::makeRef(this->_instances.data(), static_cast<uint32_t>(this->_instances.size()) * rawrbox::Instance::vLayout().getStride());
		bgfx::update(this->_dataBuffer, 0, mem);
	}

	void InstancedModel::draw() {
		if (this->_instances.empty()) return;
		if ((BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported) == 0) throw std::runtime_error("[RawrBox-InstancedModel] Instancing not supported by the graphics card!");

		rawrbox::ModelBase::draw();
		this->_material->process(*this->_mesh); // Set atlas

		if (this->isDynamic()) {
			bgfx::setVertexBuffer(0, this->_vbdh);
			bgfx::setIndexBuffer(this->_ibdh);
		} else {
			bgfx::setVertexBuffer(0, this->_vbh);
			bgfx::setIndexBuffer(this->_ibh);
		}

		// Set instance data buffer.
		bgfx::setTransform((this->getMatrix()).data());
		bgfx::setBuffer(rawrbox::SAMPLE_INSTANCE_DATA, this->_dataBuffer, bgfx::Access::Read);
		bgfx::setInstanceDataBuffer(this->_dataBuffer, 0, static_cast<uint32_t>(this->_instances.size()));
		// ----

		uint64_t flags = BGFX_STATE_DEFAULT_3D | this->_mesh->culling | this->_mesh->blending | this->_mesh->depthTest;
		flags |= this->_mesh->lineMode ? BGFX_STATE_PT_LINES : this->_mesh->wireframe ? BGFX_STATE_PT_LINESTRIP
											      : 0;

		bgfx::setState(flags, 0);
		this->_material->postProcess();
	}
} // namespace rawrbox
