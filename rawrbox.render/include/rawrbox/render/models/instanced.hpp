#pragma once

#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/models/instance.hpp>
#include <rawrbox/render/models/model.hpp>

#include <DynamicBuffer.hpp>

#include <numeric>
#include <optional>

namespace rawrbox {

	template <typename M = rawrbox::MaterialInstanced>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	class InstancedModel : public rawrbox::ModelBase<M> {
	protected:
		std::unique_ptr<Diligent::DynamicBuffer> _dataBuffer = nullptr;
		std::vector<rawrbox::Instance> _instances = {};

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _directIndices;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _shadowIndices;

		rawrbox::BBOX _instanceBounds = {};
		uint32_t _capacity = 0;

		static constexpr uint32_t SHADOW_LISTS = 2U; // Index lists per instance, enough for every instance to be in reach of two shadow lights

		void updateBuffers() override {
			rawrbox::ModelBase<M>::updateBuffers();
			this->updateInstances();
		}

		// CULLING ---
		void createCullBuffers(const uint32_t items, const uint32_t draws) override {
			rawrbox::ModelBase<M>::createCullBuffers(items, draws);
			auto* device = rawrbox::RENDERER->device();

			RAWRBOX_DESTROY(this->_directIndices);
			RAWRBOX_DESTROY(this->_shadowIndices);

			// DIRECT INDICES ---
			std::vector<uint32_t> indices(std::max(items, 1U));
			std::iota(indices.begin(), indices.end(), 0U);

			Diligent::BufferDesc indicesDesc;
			indicesDesc.Name = "RawrBox::Buffer::Instance::Direct";
			indicesDesc.Usage = Diligent::USAGE_IMMUTABLE;
			indicesDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			indicesDesc.Size = indices.size() * sizeof(uint32_t);

			const Diligent::BufferData indicesData(indices.data(), indicesDesc.Size);
			device->CreateBuffer(indicesDesc, &indicesData, &this->_directIndices);
			if (this->_directIndices == nullptr) RAWRBOX_CRITICAL("Failed to create direct instance indices");
			// -----------

			// SHADOW INDICES (per light lists, filled by the shadow plugin) ---
			Diligent::BufferDesc shadowDesc;
			shadowDesc.Name = "RawrBox::Buffer::Instance::Shadow";
			shadowDesc.Usage = Diligent::USAGE_DEFAULT;
			shadowDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			shadowDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_UNORDERED_ACCESS;
			shadowDesc.ElementByteStride = sizeof(uint32_t);
			shadowDesc.Size = indices.size() * SHADOW_LISTS * sizeof(uint32_t);

			device->CreateBuffer(shadowDesc, nullptr, &this->_shadowIndices);
			if (this->_shadowIndices == nullptr) RAWRBOX_CRITICAL("Failed to create shadow instance indices");
			// -----------

			this->_capacity = items;
		}

		virtual void cullInstances(rawrbox::CullPass mode) {
			for (uint32_t draw = 0; draw < 2; draw++) {
				const size_t record = static_cast<size_t>(draw) * this->DRAW_ARGS;

				this->_cullArgs[record] = this->_mesh->totalIndex;
				this->_cullArgs[record + 2] = this->_mesh->baseIndex;
				this->_cullArgs[record + 3] = this->_mesh->baseVertex;
			}

			this->cullObjects(this->_dataBuffer->GetBuffer(), static_cast<uint32_t>(this->_instances.size()), this->getCachedVertexBounds(), mode);
		}

		virtual void drawInstances(bool indirect, uint64_t argsOffset) {
			auto* context = rawrbox::RENDERER->context();
			if (context == nullptr) RAWRBOX_CRITICAL("Invalid renderer context");

			this->bindBuffers();

			if (indirect) { // For billboards / transparents
				constexpr uint64_t offset = 0;
				Diligent::IBuffer* visible = this->_visibleBuffer;

				context->SetVertexBuffers(1, 1, &visible, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);
			}

			// BIND MATERIALS + TEXTURES ----
			rawrbox::RENDERER->getPassCamera()->setModelTransform(this->getMatrix());

			this->_material->bindPipeline(*this->_mesh);

			rawrbox::BindlessManager::instanceBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Instances")->Set(this->_dataBuffer->GetBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
			context->CommitShaderResources(rawrbox::BindlessManager::instanceBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			this->_material->resetUniformBinds();

			bool updated = this->_material->bindVertexUniforms(*this->_mesh);
			updated |= this->_material->bindVertexSkinnedUniforms(*this->_mesh);
			updated |= this->_material->bindPixelUniforms(*this->_mesh);
			// -----------

			auto flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Instanced buffers are only updated once
			if (!updated) flags |= Diligent::DRAW_FLAG_DYNAMIC_RESOURCE_BUFFERS_INTACT;

			if (indirect) {
				Diligent::DrawIndexedIndirectAttribs attribs;

				attribs.pAttribsBuffer = this->_argsBuffer;
				attribs.DrawArgsOffset = argsOffset;
				attribs.IndexType = Diligent::VT_UINT32;
				attribs.Flags = flags;
				attribs.AttribsBufferStateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

				context->DrawIndexedIndirect(attribs);
				return;
			}

			Diligent::DrawIndexedAttribs DrawAttrs;
			DrawAttrs.IndexType = Diligent::VT_UINT32;
			DrawAttrs.FirstIndexLocation = this->_mesh->baseIndex;
			DrawAttrs.BaseVertex = this->_mesh->baseVertex;
			DrawAttrs.NumIndices = this->_mesh->totalIndex;
			DrawAttrs.NumInstances = static_cast<uint32_t>(this->_instances.size());
			DrawAttrs.Flags = flags;

			context->DrawIndexed(DrawAttrs);
		}
		// -----------

	public:
		explicit InstancedModel(size_t instanceSize = 0) {
			this->_cullCompact = true;
			if (instanceSize != 0) this->_instances.reserve(instanceSize);
		}

		InstancedModel(const InstancedModel&) = delete;
		InstancedModel(InstancedModel&&) = delete;
		InstancedModel& operator=(const InstancedModel&) = delete;
		InstancedModel& operator=(InstancedModel&&) = delete;
		~InstancedModel() override {
			RAWRBOX_DESTROY(this->_directIndices);
			RAWRBOX_DESTROY(this->_shadowIndices);

			this->_dataBuffer.reset();
			this->_instances.clear();
		}

		virtual void setTemplate(rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			if (mesh.empty()) RAWRBOX_CRITICAL("Invalid mesh! Missing vertices / indices!");
			this->_mesh = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh);

			if (this->isUploaded() && this->isDynamic()) {
				this->updateBuffers();
			}
		}

		[[nodiscard]] virtual rawrbox::Mesh<typename M::vertexBufferType>& getTemplate() const {
			if (this->_mesh == nullptr) RAWRBOX_CRITICAL("Invalid mesh! Missing vertices / indices!");
			return *this->_mesh;
		}

		virtual void addInstance(const rawrbox::Instance& instance) {
			this->_instances.push_back(instance);
		}

		virtual void removeInstance(size_t i = 0) {
			if (i >= this->_instances.size()) RAWRBOX_CRITICAL("Failed to find instance");
			this->_instances.erase(this->_instances.begin() + i);
		}

		[[nodiscard]] rawrbox::Instance& getInstance(size_t i = 0) {
			if (i >= this->_instances.size()) RAWRBOX_CRITICAL("Failed to find instance");
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
			InstBuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			InstBuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

			Diligent::DynamicBufferCreateInfo dynamicBuff;
			dynamicBuff.Desc = InstBuffDesc;

			this->_dataBuffer = std::make_unique<Diligent::DynamicBuffer>(device, dynamicBuff);
			this->_dataBuffer->Resize(device, rawrbox::RENDERER->context(), sizeof(rawrbox::Instance) * std::max<uint64_t>(this->_instances.capacity(), 1U), true);
			//  ---------------------

			// Barrier ----
			rawrbox::BarrierUtils::barrier({{this->_dataBuffer->GetBuffer(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			// ------------

			this->createCullBuffers(static_cast<uint32_t>(this->_instances.capacity()), 2U);
			if (size != 0) this->updateInstances(); // Data was already added, then update the buffer
		}

		virtual void updateInstances(std::optional<size_t> first = std::nullopt, std::optional<size_t> count = std::nullopt) {
			if (this->_dataBuffer == nullptr) RAWRBOX_CRITICAL("Data buffer not valid! Did you call upload()?");

			auto* context = rawrbox::RENDERER->context();
			if (context == nullptr) RAWRBOX_CRITICAL("Invalid renderer context");

			auto* device = rawrbox::RENDERER->device();
			if (device == nullptr) RAWRBOX_CRITICAL("Invalid renderer device");

			size_t start = first.value_or(0);
			if (start > this->_instances.size()) RAWRBOX_CRITICAL("Instance range out of bounds");

			size_t total = count.value_or(this->_instances.size() - start);
			if (start + total > this->_instances.size()) RAWRBOX_CRITICAL("Instance range out of bounds");

			// RESIZE ----
			if (this->_instances.size() > this->_capacity) {
				this->_instances.reserve(this->_instances.size() + 16);

				const auto capacity = static_cast<uint32_t>(this->_instances.capacity());
				this->_dataBuffer->Resize(device, context, sizeof(rawrbox::Instance) * static_cast<uint64_t>(capacity), true);
				this->createCullBuffers(capacity, 2U);

				start = 0;
				total = this->_instances.size();
			}
			//  ---------

			const bool full = total == this->_instances.size();
			if (full) this->_resetVIS = true;

			// SHADOW BOUNDS ---
			const auto& local = this->getCachedVertexBounds();

			if (!local.isInfinite()) {
				for (size_t i = start; i < start + total; i++) {
					const auto bounds = local.transform(this->_instances[i].getMatrix());

					if (full && i == start) {
						this->_instanceBounds = bounds;
					} else {
						this->_instanceBounds.combine(bounds);
					}
				}
			}
			//  ---------

			if (total == 0) return;
			context->UpdateBuffer(this->_dataBuffer->GetBuffer(), sizeof(rawrbox::Instance) * start, sizeof(rawrbox::Instance) * total, &this->_instances[start], Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}

		void bindBuffers() override {
			auto* context = rawrbox::RENDERER->context();
			if (context == nullptr) RAWRBOX_CRITICAL("Invalid renderer context");

			// Bind vertex and index buffers
			constexpr std::array<uint64_t, 2> offset = {0, 0};
			const std::array<Diligent::IBuffer*, 2> pBuffs = {this->_vbh, this->_directIndices};

			context->SetVertexBuffers(0, 2, pBuffs.data(), offset.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(this->_ibh, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			// ----
		}

		// SHADOWS ---
		[[nodiscard]] rawrbox::BBOX getShadowBounds() override {
			if (this->_instances.empty() || this->getCachedVertexBounds().isInfinite()) return rawrbox::BBOX::infinite();
			return this->_instanceBounds.transform(this->getMatrix());
		}

		[[nodiscard]] bool getShadowInstances(rawrbox::ShadowInstances& instances) override {
			if (this->_instances.empty() || !this->canCastShadows(*this->_mesh) || this->getCachedVertexBounds().isInfinite()) return false;

			instances = {
			    .instances = this->_dataBuffer->GetBuffer(),
			    .indices = this->_shadowIndices,
			    .matrix = this->getMatrix(),
			    .bounds = this->getCachedVertexBounds(),
			    .total = static_cast<uint32_t>(this->_instances.size()),
			    .capacity = static_cast<uint32_t>(this->_shadowIndices->GetDesc().Size / sizeof(uint32_t))};

			return true;
		}

		void drawShadow(const rawrbox::ShadowDraw& draw) override {
			if (this->_instances.empty() || !this->canCastShadows(*this->_mesh)) return;

			if (draw.bind) {
				this->bindBuffers();

				if (draw.culled) {
					constexpr uint64_t offset = 0;
					Diligent::IBuffer* indices = this->_shadowIndices;

					rawrbox::RENDERER->context()->SetVertexBuffers(1, 1, &indices, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_NONE);
				}
				this->_material->resetUniformBinds();
				this->bindShadowMesh(*this->_mesh, this->getMatrix());

				rawrbox::BindlessManager::instanceBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "Instances")->Set(this->_dataBuffer->GetBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
				rawrbox::RENDERER->context()->CommitShaderResources(rawrbox::BindlessManager::instanceBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			}

			const auto bounds = this->getShadowBounds();
			for (uint32_t view = 0; view < draw.views; view++) {
				draw.setView(view);
				draw.submit({bounds, this->_mesh->totalIndex, this->_mesh->baseIndex, this->_mesh->baseVertex, static_cast<uint32_t>(this->_instances.size()), view});
			}
		}
		// -----------

		void draw() override {
			if (!this->isUploaded()) RAWRBOX_CRITICAL("Failed to render model, vertex / index buffer is not uploaded");
			if (this->_instances.empty()) return;

			// OCCLUSION ---
			auto* occlusion = rawrbox::RENDERER->occlusion();
			if (occlusion == nullptr) RAWRBOX_CRITICAL("Invalid occlusion pass");

			const bool recording = occlusion->isRecording();

			if (!this->_mesh->isCullable() || this->_mesh->isTransparent() || this->getCachedVertexBounds().isInfinite()) {
				if (recording && this->_mesh->isTransparent()) {
					occlusion->queue(*this);
				} else {
					this->drawInstances(false, 0);
				}

				return;
			}

			this->cullInstances(recording ? rawrbox::CullPass::EARLY : rawrbox::CullPass::FRUSTUM);
			this->drawInstances(true, 0);

			if (recording) occlusion->queue(*this);
			// -----------
		}

		void drawLate() override {
			if (!this->_mesh->isCullable() || this->_mesh->isTransparent() || this->getCachedVertexBounds().isInfinite()) {
				this->drawInstances(false, 0);
				return;
			}

			this->cullInstances(rawrbox::CullPass::LATE);
			this->drawInstances(true, this->DRAW_ARGS * sizeof(uint32_t));
		}
	};
} // namespace rawrbox