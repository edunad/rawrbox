
#include <rawrbox/render/models/instanced.hpp>
#include <rawrbox/render/renderers/base.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/scripting/scripting.hpp>
#endif

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

	void InstancedModel::upload(bool dynamic) {
		rawrbox::ModelBase::upload(dynamic);
		auto device = rawrbox::RENDERER->device;

		auto instSize = static_cast<uint32_t>(this->_instances.size());

		// INSTANCE BUFFER ----
		Diligent::BufferDesc InstBuffDesc;
		InstBuffDesc.Name = "RawrBox::Buffer::Instance";
		InstBuffDesc.Usage = Diligent::USAGE_DEFAULT;
		InstBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
		InstBuffDesc.Size = instSize * sizeof(rawrbox::Instance);

		Diligent::BufferData VBData;
		VBData.pData = this->_instances.data();
		VBData.DataSize = InstBuffDesc.Size;

		device->CreateBuffer(InstBuffDesc, this->_instances.empty() ? nullptr : &VBData, &this->_dataBuffer);
		// ---------------------
	}

	void InstancedModel::updateInstance() {
		if (this->_dataBuffer == nullptr) throw std::runtime_error("[RawrBox-InstancedModel] Data buffer not valid! Did you call upload()?");

		auto context = rawrbox::RENDERER->context;
		auto instSize = static_cast<uint32_t>(this->_instances.size());

		context->UpdateBuffer(this->_vbh, 0, sizeof(rawrbox::Instance) * instSize, this->_instances.empty() ? nullptr : this->_instances.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	void InstancedModel::draw() {
		if (!this->isUploaded()) throw std::runtime_error("[RawrBox-Model] Failed to render model, vertex / index buffer is not uploaded");
		if (this->_instances.empty()) return;

		auto context = rawrbox::RENDERER->context;

		// Bind vertex and index buffers
		// NOLINTBEGIN(*)
		const uint64_t offset[] = {0, 0};
		Diligent::IBuffer* pBuffs[] = {this->_vbh, this->_dataBuffer};
		// NOLINTEND(*)

		context->SetVertexBuffers(0, 2, pBuffs, offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
		context->SetIndexBuffer(this->_ibh, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		// ----

		// Bind materials uniforms & textures ----
		rawrbox::TRANSFORM = this->getMatrix() * this->_mesh->getMatrix();
		this->_material->bind(*this->_mesh);
		// -----------

		Diligent::DrawIndexedAttribs DrawAttrs;    // This is an indexed draw call
		DrawAttrs.IndexType = Diligent::VT_UINT16; // Index type
		DrawAttrs.FirstIndexLocation = this->_mesh->baseIndex;
		DrawAttrs.BaseVertex = this->_mesh->baseVertex;
		DrawAttrs.NumIndices = this->_mesh->totalIndex;
		DrawAttrs.NumInstances = static_cast<uint32_t>(this->_instances.size());
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
		context->DrawIndexed(DrawAttrs);
	}
} // namespace rawrbox
