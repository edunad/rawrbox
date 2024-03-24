#pragma once

#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/models/instance.hpp>
#include <rawrbox/render/models/model.hpp>

namespace rawrbox {

	template <typename M = rawrbox::MaterialInstanced>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	class InstancedModel : public rawrbox::ModelBase<M> {
	protected:
		std::unique_ptr<Diligent::DynamicBuffer> _dataBuffer = nullptr;
		std::vector<rawrbox::Instance> _instances = {};

		void updateBuffers() override {
			rawrbox::ModelBase<M>::updateBuffers();
			this->updateInstances();
		}

	public:
		explicit InstancedModel(size_t instanceSize = 0) {
			if (instanceSize != 0) this->_instances.reserve(instanceSize);
		}

		InstancedModel(const InstancedModel&) = delete;
		InstancedModel(InstancedModel&&) = delete;
		InstancedModel& operator=(const InstancedModel&) = delete;
		InstancedModel& operator=(InstancedModel&&) = delete;
		~InstancedModel() override {
			this->_dataBuffer.reset();
			this->_instances.clear();
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
		}

		virtual void removeInstance(size_t i = 0) {
			if (i < 0 || i >= this->_instances.size()) throw this->_logger->error("Failed to find instance");
			this->_instances.erase(this->_instances.begin() + i);
		}

		[[nodiscard]] rawrbox::Instance& getInstance(size_t i = 0) {
			if (i < 0 || i >= this->_instances.size()) throw this->_logger->error("Failed to find instance");
			return this->_instances[i];
		}

		virtual std::vector<rawrbox::Instance>& instances() { return this->_instances; }
		[[nodiscard]] virtual size_t count() const { return this->_instances.size(); }

		void upload(rawrbox::UploadType type = rawrbox::UploadType::STATIC) override {
			rawrbox::ModelBase<M>::upload(type);

			auto* device = rawrbox::RENDERER->device();
			auto size = this->_instances.size();

			// INSTANCE BUFFER ----
			Diligent::BufferDesc InstBuffDesc;
			InstBuffDesc.ElementByteStride = sizeof(rawrbox::Instance);
			InstBuffDesc.Name = "RawrBox::Buffer::Instance";
			InstBuffDesc.Usage = Diligent::USAGE_SPARSE;
			InstBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;

			Diligent::DynamicBufferCreateInfo dynamicBuff;
			dynamicBuff.Desc = InstBuffDesc;

			this->_dataBuffer = std::make_unique<Diligent::DynamicBuffer>(device, dynamicBuff);
			//  ---------------------

			// Barrier ----
			rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{this->_dataBuffer->GetBuffer(), Diligent::RESOURCE_STATE_VERTEX_BUFFER}});
			// ------------

			if (size != 0) this->updateInstances(); // Data was already added, then update the buffer
		}

		virtual void updateInstances() {
			if (this->_dataBuffer == nullptr) throw this->_logger->error("Data buffer not valid! Did you call upload()?");

			auto* context = rawrbox::RENDERER->context();
			auto* device = rawrbox::RENDERER->device();

			// Update buffer ----
			uint64_t size = sizeof(rawrbox::Instance) * static_cast<uint64_t>(this->_instances.size());
			if (size > this->_dataBuffer->GetDesc().Size) this->_dataBuffer->Resize(device, context, (size + 15) & ~15); // + OFFSET

			auto* buffer = this->_dataBuffer->GetBuffer();

			// BARRIER ----
			rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{buffer, Diligent::RESOURCE_STATE_COPY_DEST}});
			context->UpdateBuffer(buffer, 0, size, this->_instances.empty() ? nullptr : this->_instances.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{buffer, Diligent::RESOURCE_STATE_VERTEX_BUFFER}});
			//  ---------
		}

		void draw() override {
			if (!this->isUploaded()) throw this->_logger->error("Failed to render model, vertex / index buffer is not uploaded");
			if (this->_instances.empty()) return;

			auto* context = rawrbox::RENDERER->context();

			// Bind vertex and index buffers
			// NOLINTBEGIN(*)
			const uint64_t offset[] = {0, 0};
			Diligent::IBuffer* pBuffs[] = {this->_vbh, this->_dataBuffer->GetBuffer()};
			// NOLINTEND(*)

			context->SetVertexBuffers(0, 2, pBuffs, offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(this->_ibh, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			// ----

			// Bind materials uniforms & textures ----
			rawrbox::MAIN_CAMERA->setModelTransform(this->getMatrix());

			this->_material->bindPipeline(*this->_mesh);

			// bool updated = false;
			// updated = this->_material->bindVertexUniforms(*this->_mesh);
			// updated = this->_material->bindVertexSkinnedUniforms(*this->_mesh);
			// updated = this->_material->bindPixelUniforms(*this->_mesh);

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
			DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Instanced buffers are only updated once
			// if (!updated) DrawAttrs.Flags |= Diligent::DRAW_FLAG_DYNAMIC_RESOURCE_BUFFERS_INTACT;

			context->DrawIndexed(DrawAttrs);
		}
	};
} // namespace rawrbox
