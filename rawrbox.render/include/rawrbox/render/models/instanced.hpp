#pragma once

#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/models/instance.hpp>
#include <rawrbox/render/models/model.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/render/scripting/wrappers/model/instanced_wrapper.hpp>
	#include <sol/sol.hpp>
#endif

namespace rawrbox {

	template <typename M = rawrbox::MaterialInstanced>
	class InstancedModel : public rawrbox::ModelBase<M> {
	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _dataBuffer; // Data
		std::vector<rawrbox::Instance> _instances = {};
		bool _autoUpload = true;

		void updateBuffers() override {
			rawrbox::ModelBase<M>::updateBuffers();
			this->updateInstance();
		}

#ifdef RAWRBOX_SCRIPTING
		void initializeLua() override {
			if (this->_luaWrapper.valid()) this->_luaWrapper.abandon();
			this->_luaWrapper = sol::make_object(rawrbox::SCRIPTING::getLUA(), rawrbox::InstancedModelWrapper(this->shared_from_this()));
		}
#endif

	public:
		explicit InstancedModel(size_t instanceSize = 0) {
			if (instanceSize != 0) this->_instances.reserve(instanceSize);
		}

		InstancedModel(const InstancedModel&) = delete;
		InstancedModel(InstancedModel&&) = delete;
		InstancedModel& operator=(const InstancedModel&) = delete;
		InstancedModel& operator=(InstancedModel&&) = delete;
		~InstancedModel() override {
			RAWRBOX_DESTROY(this->_dataBuffer);
			this->_instances.clear();
		}

		virtual void setAutoUpload(bool enabled) {
			this->_autoUpload = enabled;
		}

		virtual void setTemplate(rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			if (mesh.empty()) throw this->_logger->error("Invalid mesh! Missing vertices / indices!");
			this->_mesh = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh);

			if (this->isUploaded() && this->isDynamic()) {
				this->updateBuffers();
			}
		}

		[[nodiscard]] virtual rawrbox::Mesh<typename M::vertexBufferType>& getTemplate() const {
			if (this->_mesh == nullptr) throw this->_logger->error("Invalid mesh! Missing vertices / indices!");
			return *this->_mesh;
		}

		virtual void addInstance(const rawrbox::Instance& instance) {
			this->_instances.push_back(instance);
			if (this->isUploaded() && this->_autoUpload) this->updateInstance();
		}

		virtual void removeInstance(size_t i = 0) {
			if (i < 0 || i >= this->_instances.size()) throw this->_logger->error("Failed to find instance");
			this->_instances.erase(this->_instances.begin() + i);

			if (this->isUploaded() && this->_autoUpload) this->updateInstance();
		}

		[[nodiscard]] rawrbox::Instance& getInstance(size_t i = 0) {
			if (i < 0 || i >= this->_instances.size()) throw this->_logger->error("Failed to find instance");
			return this->_instances[i];
		}

		virtual std::vector<rawrbox::Instance>& instances() { return this->_instances; }
		[[nodiscard]] virtual size_t count() const { return this->_instances.size(); }

		void upload(bool dynamic = false) override {
			rawrbox::ModelBase<M>::upload(dynamic);

			auto device = rawrbox::RENDERER->device();
			auto context = rawrbox::RENDERER->context();

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

			// Barrier ----
			rawrbox::BindlessManager::barrier(*this->_dataBuffer, rawrbox::BufferType::CONSTANT);
			// ------------
		}

		virtual void updateInstance() {
			if (this->_dataBuffer == nullptr) throw this->_logger->error("Data buffer not valid! Did you call upload()?");

			auto context = rawrbox::RENDERER->context();
			auto instSize = static_cast<uint32_t>(this->_instances.size());

			context->UpdateBuffer(this->_vbh, 0, sizeof(rawrbox::Instance) * instSize, this->_instances.empty() ? nullptr : this->_instances.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			rawrbox::BindlessManager::barrier(*this->_vbh, rawrbox::BufferType::VERTEX);
		}

		void draw() override {
			if (!this->isUploaded()) throw this->_logger->error("Failed to render model, vertex / index buffer is not uploaded");
			if (this->_instances.empty()) return;

			auto context = rawrbox::RENDERER->context();

			// Bind vertex and index buffers
			// NOLINTBEGIN(*)
			const uint64_t offset[] = {0, 0};
			Diligent::IBuffer* pBuffs[] = {this->_vbh, this->_dataBuffer};
			// NOLINTEND(*)

			context->SetVertexBuffers(0, 2, pBuffs, offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(this->_ibh, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			// ----

			// Bind materials uniforms & textures ----
			rawrbox::MAIN_CAMERA->setModelTransform(this->getMatrix());

			this->_material->bindPipeline(*this->_mesh);

			this->_material->resetUniformBinds();
			this->_material->bindVertexUniforms(*this->_mesh);
			this->_material->bindVertexSkinnedUniforms(*this->_mesh);
			this->_material->bindPixelUniforms(*this->_mesh);
			// -----------

			Diligent::DrawIndexedAttribs DrawAttrs;
			DrawAttrs.IndexType = Diligent::VT_UINT16;
			DrawAttrs.FirstIndexLocation = this->_mesh->baseIndex;
			DrawAttrs.BaseVertex = this->_mesh->baseVertex;
			DrawAttrs.NumIndices = this->_mesh->totalIndex;
			DrawAttrs.NumInstances = static_cast<uint32_t>(this->_instances.size());
			DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL | Diligent::DRAW_FLAG_DYNAMIC_RESOURCE_BUFFERS_INTACT; // Instanced buffers are only updated once
			context->DrawIndexed(DrawAttrs);
		}
	};
} // namespace rawrbox
