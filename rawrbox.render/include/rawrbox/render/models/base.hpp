#pragma once

#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/occlusion.hpp>
#include <rawrbox/render/shadows/caster.hpp>
#include <rawrbox/render/utils/pipeline.hpp>
#include <rawrbox/utils/dirty_ranges.hpp>
#include <rawrbox/utils/string.hpp>

#include <Buffer.h>

namespace rawrbox {

	template <typename M = rawrbox::MaterialUnlit>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	struct BlendShapes {
	public:
		float weight = 0.F;

		rawrbox::Mesh<typename M::vertexBufferType>* mesh = nullptr; // For quick access

		std::vector<rawrbox::Vector3f> pos = {};
		std::vector<rawrbox::Vector4f> normals = {};

		[[nodiscard]] bool isActive() const { return weight > 0.F; }
		BlendShapes() = default;
	};

	enum class UploadType {
		STATIC = 0,
		FIXED_DYNAMIC = 1,
		RESIZABLE_DYNAMIC = 2
	};

	enum class CullPass {
		FRUSTUM = 0,
		EARLY,
		LATE,

		COUNT
	};

	struct ModelOriginalData {
		rawrbox::Vector3f pos = {};
		uint32_t normal = {};
	};

	template <typename M = rawrbox::MaterialUnlit>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	class ModelBase : public rawrbox::ShadowCaster, public rawrbox::OcclusionDraw { // Meh

	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _vbh; // Vertices
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _ibh; // Indices

		std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>> _mesh = nullptr;
		std::unique_ptr<M> _material = nullptr;

		std::unordered_map<std::string, std::unique_ptr<rawrbox::BlendShapes<M>>> _blend_shapes = {};
		std::vector<rawrbox::ModelOriginalData> _original_data = {};

		// DYNAMIC SUPPORT ---
		rawrbox::UploadType _uploadType = rawrbox::UploadType::STATIC;

		rawrbox::DirtyRanges _dirtyVB = {};
		rawrbox::DirtyRanges _dirtyIB = {};

		bool _requiresUpdate = false;
		// ----

		// SHADOWS ---
		rawrbox::BBOX _vertexBounds = {};
		bool _vertexBoundsDirty = true;
		// ----

		// GPU CULLING ---
		static constexpr uint32_t DRAW_ARGS = 5U;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _visibleBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _visibility;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _argsBuffer;

		std::vector<uint32_t> _cullArgs = {};

		bool _cullCompact = false;
		bool _resetVIS = true;
		// ----

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Model");
		// -------------

		// BLEND SHAPES ---
		virtual void calculateBlendShapes() {
			const auto vertSize = static_cast<uint32_t>(this->_mesh->vertices.size());
			const auto indcSize = static_cast<uint32_t>(this->_mesh->indices.size());

			auto empty = vertSize == 0 || indcSize == 0;
			if (empty) RAWRBOX_CRITICAL("Invalid mesh! Missing vertices / indices!");

			// Store original positions for blend states
			this->_original_data.clear();
			this->_original_data.reserve(vertSize);

			for (auto& v : this->_mesh->vertices) {
				if constexpr (supportsNormals<typename M::vertexBufferType>) {
					this->_original_data.push_back({v.position, v.normal});
				} else {
					this->_original_data.push_back({v.position, {}});
				}
			}
			// -----------
		}

		virtual void applyBlendShapes() {
			if (!this->isUploaded()) RAWRBOX_CRITICAL("Blendshapes require the model to be uploaded!");
			if (!this->isDynamic()) RAWRBOX_CRITICAL("Blendshapes require the model to be dynamic!");

			if (this->_original_data.empty()) this->calculateBlendShapes(); // Initial setup

			for (auto& shape : this->_blend_shapes) {
				if (!shape.second->isActive() || shape.second->mesh == nullptr) continue;

				auto& verts = shape.second->mesh->vertices;
				auto& blendPos = shape.second->pos;
				auto& blendNormals = shape.second->normals;

				if (!blendPos.empty() && blendPos.size() != verts.size()) {
					RAWRBOX_CRITICAL("Blendshape verts do not match with the mesh '{}' verts! Total verts: {}, blend shape verts: {}", shape.first, verts.size(), blendPos.size());
				}

				if (!blendNormals.empty() && blendNormals.size() != verts.size()) {
					RAWRBOX_CRITICAL("Blendshape normals do not match with the mesh '{}' verts! Total verts: {}, blend shape verts: {}", shape.first, verts.size(), blendNormals.size());
				}

				float step = std::clamp(shape.second->weight, 0.F, 1.F);

				// Apply vertices ----
				for (size_t i = 0; i < blendPos.size(); i++) {
					verts[i].position = this->_original_data[i].pos.lerp(blendPos[i], step);
				}
				// -------------------

				// Apply normal ----
				if constexpr (supportsNormals<typename M::vertexBufferType>) {
					for (size_t i = 0; i < blendNormals.size(); i++) {
						const auto original = rawrbox::PackUtils::fromOCTNormal(this->_original_data[i].normal);
						const auto blended = rawrbox::Vector3f(original[0], original[1], original[2]).lerp(blendNormals[i].xyz(), step);

						verts[i].normal = rawrbox::PackUtils::packOCTNormal(blended.x, blended.y, blended.z);
					}
				}
				// -------------------
			}

			this->_vertexBoundsDirty = true;
		}
		// --------------

		virtual void internalUpdate() {
			if (!this->isUploaded() || !this->isDynamic() || !this->_requiresUpdate) return;
			auto* context = rawrbox::RENDERER->context();
			if (context == nullptr) RAWRBOX_CRITICAL("Context is not set!");

			const auto vertSize = static_cast<uint64_t>(this->_mesh->vertices.size());
			const auto indcSize = static_cast<uint64_t>(this->_mesh->indices.size());

			const bool empty = vertSize == 0 || indcSize == 0;

			uint64_t sizeVB = sizeof(typename M::vertexBufferType) * vertSize;
			uint64_t sizeIB = sizeof(uint32_t) * indcSize;

			bool resizable = this->_uploadType == rawrbox::UploadType::RESIZABLE_DYNAMIC;

			const bool resizeVertex = resizable && sizeVB > this->_vbh->GetDesc().Size;
			const bool resizeIndex = resizable && sizeIB > this->_ibh->GetDesc().Size;

			if (resizeVertex) {
				RAWRBOX_DESTROY(this->_vbh);

				this->_mesh->vertices.reserve(vertSize + RB_RENDER_BUFFER_INCREASE_OFFSET);
				this->createVertexBuffer();
				this->_dirtyVB.clear();

				this->_logger->debug("Resizing vertex buffer ({} -> {})", fmt::styled(vertSize, fmt::fg(fmt::color::cyan)), fmt::styled(this->_mesh->vertices.capacity(), fmt::fg(fmt::color::cyan)));
			}

			if (resizeIndex) {
				RAWRBOX_DESTROY(this->_ibh);

				this->_mesh->indices.reserve(indcSize + RB_RENDER_BUFFER_INCREASE_OFFSET);
				this->createIndexBuffer();
				this->_dirtyIB.clear();

				this->_logger->debug("Resizing index buffer ({} -> {})", fmt::styled(indcSize, fmt::fg(fmt::color::cyan)), fmt::styled(this->_mesh->indices.capacity(), fmt::fg(fmt::color::cyan)));
			}

			if (resizeVertex && resizeIndex) {
				this->_requiresUpdate = false;
				return;
			}

			// BARRIER -----
			std::vector<Diligent::StateTransitionDesc> barriers = {};
			if (!resizeVertex) barriers.emplace_back(this->_vbh, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			if (!resizeIndex) barriers.emplace_back(this->_ibh, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

			rawrbox::BarrierUtils::barrier(barriers);

			const auto upload = [&](Diligent::IBuffer* buffer, rawrbox::DirtyRanges& dirty, const uint8_t* data, uint64_t fullSize) {
				if (dirty.full() || dirty.empty()) {
					context->UpdateBuffer(buffer, 0, fullSize, fullSize == 0 ? nullptr : data, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
					return;
				}

				for (const auto& range : dirty.merge(fullSize)) {
					context->UpdateBuffer(buffer, range.offset, range.size, data + range.offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
				}

				if (dirty.full()) {
					context->UpdateBuffer(buffer, 0, fullSize, fullSize == 0 ? nullptr : data, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
				}
			};

			if (!resizeVertex) upload(this->_vbh, this->_dirtyVB, empty ? nullptr : reinterpret_cast<const uint8_t*>(this->_mesh->vertices.data()), vertSize * sizeof(typename M::vertexBufferType));
			if (!resizeIndex) upload(this->_ibh, this->_dirtyIB, empty ? nullptr : reinterpret_cast<const uint8_t*>(this->_mesh->indices.data()), indcSize * sizeof(uint32_t));

			this->_dirtyVB.clear();
			this->_dirtyIB.clear();

			barriers.clear();
			if (!resizeVertex) barriers.emplace_back(this->_vbh, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			if (!resizeIndex) barriers.emplace_back(this->_ibh, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

			rawrbox::BarrierUtils::barrier(barriers);
			// -----------

			this->_requiresUpdate = false;
		}

		virtual void bindBuffers() {
			auto* context = rawrbox::RENDERER->context();

			// Pending updates --
			this->internalUpdate();
			// --------------------------

			// Bind vertex and index buffers
			const std::array<Diligent::IBuffer*, 1> pBuffs = {this->_vbh};

			context->SetVertexBuffers(0, 1, pBuffs.data(), nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(this->_ibh, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			// ----
		}

		[[nodiscard]] static rawrbox::BBOX getVertexBounds(const rawrbox::Mesh<typename M::vertexBufferType>& mesh) {
			if (mesh.vertices.empty() || mesh.skeleton != nullptr) return rawrbox::BBOX::infinite();

			const auto& first = mesh.vertices.front().position;
			rawrbox::BBOX bounds = {first, first, {}};

			for (const auto& vertex : mesh.vertices) {
				bounds.expand(vertex.position);
			}

			return bounds;
		}

		virtual void createIndexBuffer() {
			auto* device = rawrbox::RENDERER->device();

			const auto indcSize = static_cast<uint64_t>(std::max<size_t>(this->_mesh->indices.capacity(), this->isDynamic() ? 1U : 0U)); // RESIZABLE requires at least one vertice
			if (indcSize == 0) RAWRBOX_CRITICAL("Indices data cannot be empty!");

			Diligent::BufferDesc IndcBuffDesc;
			IndcBuffDesc.Name = "RawrBox::Buffer::Indices";
			IndcBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			IndcBuffDesc.Usage = this->isDynamic() ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE; // TODO: Diligent::USAGE_SPARSE;
			IndcBuffDesc.Size = static_cast<uint32_t>(sizeof(uint32_t)) * indcSize;
			IndcBuffDesc.ElementByteStride = sizeof(uint32_t);

			Diligent::BufferData IBData;
			IBData.pData = this->_mesh->indices.data();
			IBData.DataSize = IndcBuffDesc.Size;

			device->CreateBuffer(IndcBuffDesc, &IBData, &this->_ibh);
			if (this->_ibh == nullptr) RAWRBOX_CRITICAL("Failed to create index buffer");

			rawrbox::BarrierUtils::barrier({{this->_ibh, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		}

		// GPU CULLING ---
		virtual void createCullBuffers(const uint32_t items, const uint32_t draws) {
			auto* device = rawrbox::RENDERER->device();
			if (device == nullptr) RAWRBOX_CRITICAL("Device is not set!");

			RAWRBOX_DESTROY(this->_visibleBuffer);
			RAWRBOX_DESTROY(this->_visibility);
			RAWRBOX_DESTROY(this->_argsBuffer);

			Diligent::BufferDesc visibleDesc;
			visibleDesc.Name = "RawrBox::Buffer::Cull::Visible";
			visibleDesc.Usage = Diligent::USAGE_DEFAULT;
			visibleDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			visibleDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER | Diligent::BIND_UNORDERED_ACCESS;
			visibleDesc.ElementByteStride = sizeof(uint32_t);
			visibleDesc.Size = static_cast<uint64_t>(this->_cullCompact ? std::max(items, 1U) : 1U) * sizeof(uint32_t);

			Diligent::BufferDesc visibilityDesc;
			visibilityDesc.Name = "RawrBox::Buffer::Cull::Visibility";
			visibilityDesc.Usage = Diligent::USAGE_DEFAULT;
			visibilityDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			visibilityDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS;
			visibilityDesc.ElementByteStride = sizeof(uint32_t);
			visibilityDesc.Size = static_cast<uint64_t>(std::max(items, 1U)) * sizeof(uint32_t);

			Diligent::BufferDesc argsDesc;
			argsDesc.Name = "RawrBox::Buffer::Cull::Args";
			argsDesc.Usage = Diligent::USAGE_DEFAULT;
			argsDesc.Mode = Diligent::BUFFER_MODE_RAW;
			argsDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_INDIRECT_DRAW_ARGS;
			argsDesc.ElementByteStride = sizeof(uint32_t);
			argsDesc.Size = static_cast<uint64_t>(std::max(draws, 1U)) * DRAW_ARGS * sizeof(uint32_t);

			device->CreateBuffer(visibleDesc, nullptr, &this->_visibleBuffer);
			device->CreateBuffer(visibilityDesc, nullptr, &this->_visibility);
			device->CreateBuffer(argsDesc, nullptr, &this->_argsBuffer);
			if (this->_visibleBuffer == nullptr || this->_visibility == nullptr || this->_argsBuffer == nullptr)
				RAWRBOX_CRITICAL("Failed to create culling buffers");

			this->_cullArgs.assign(static_cast<size_t>(std::max(draws, 1U)) * DRAW_ARGS, 0U);
			this->_resetVIS = true;
		}

		virtual void cullObjects(Diligent::IBuffer* objects, uint32_t total, const rawrbox::BBOX& bounds, const rawrbox::CullPass mode) {
			if (this->_visibility == nullptr) RAWRBOX_CRITICAL("Culling buffers not created!");
			if (total == 0) return;

			auto* context = rawrbox::RENDERER->context();
			if (context == nullptr) RAWRBOX_CRITICAL("Context is not initialized!");

			const auto* occlusion = rawrbox::RENDERER->occlusion();
			if (occlusion == nullptr) RAWRBOX_CRITICAL("occlusion is not initialized!");

			const uint32_t groups = rawrbox::MathUtils::divideRound(total, rawrbox::Occlusion::CULL_THREADS);
			if (groups > rawrbox::Occlusion::CULL_MAX_GROUPS)
				RAWRBOX_CRITICAL("Too many items to cull ({}), max is {}", total, rawrbox::Occlusion::CULL_MAX_GROUPS * rawrbox::Occlusion::CULL_THREADS);

			auto* bind = occlusion->getCullBind();

			// BUFFERS ----
			if (this->_visibility->GetDesc().Size < static_cast<uint64_t>(total) * sizeof(uint32_t))
				RAWRBOX_CRITICAL("Culling buffers are smaller than the items! Recreate buffers first");

			bind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Instances")->Set(objects->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
			bind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "VisibleInstances")->Set(this->_visibleBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
			bind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Visibility")->Set(this->_visibility->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
			bind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "DrawArgs")->Set(this->_argsBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS));
			bind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "HiZ")->Set(occlusion->getHiZ());
			// -----------

			// CONSTANTS ----
			const auto clip = rawrbox::RENDERER->getPassCamera()->getViewProjMtx() * this->getMatrix();

			rawrbox::CullConstants constants = {};
			for (uint32_t i = 0; i < 4; i++) {
				constants.clip[i] = {clip[i], clip[4 + i], clip[8 + i], clip[12 + i]};
			}

			constants.center = {(bounds.min + bounds.max) * 0.5F, 0.F};
			constants.extents = {bounds.size * 0.5F, 0.F};
			constants.settings = {total, mode == rawrbox::CullPass::LATE ? 1U : 0U, this->_resetVIS || mode == rawrbox::CullPass::FRUSTUM ? 1U : 0U, occlusion->getMips()};
			constants.hiz = {occlusion->getSize().x, occlusion->getSize().y, this->_cullCompact ? 0U : 1U, 0U};

			// Re-use the buffer, so AMD stops whining
			bind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "CullConstants")->SetInlineConstants(&constants, 0, sizeof(rawrbox::CullConstants) / sizeof(uint32_t));

			if (mode == rawrbox::CullPass::LATE) this->_resetVIS = false;
			if (mode != rawrbox::CullPass::LATE && this->_cullCompact) context->UpdateBuffer(this->_argsBuffer, 0, this->_cullArgs.size() * sizeof(uint32_t), this->_cullArgs.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			// -----------

			context->SetPipelineState(occlusion->getCullProgram());
			context->CommitShaderResources(bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

			Diligent::DispatchComputeAttribs attribs;
			attribs.ThreadGroupCountX = groups;
			context->DispatchCompute(attribs);
		}
		// -----------

		virtual void createVertexBuffer() {
			auto* device = rawrbox::RENDERER->device();

			const auto vertSize = static_cast<uint64_t>(std::max<size_t>(this->_mesh->vertices.capacity(), this->isDynamic() ? 1U : 0U)); // RESIZABLE requires at least one vertice
			if (vertSize == 0) RAWRBOX_CRITICAL("Vertices data cannot be empty!");

			Diligent::BufferDesc VertBuffDesc;
			VertBuffDesc.Name = "RawrBox::Buffer::Vertex";
			VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			VertBuffDesc.Usage = this->isDynamic() ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE; // TODO:  Diligent::USAGE_SPARSE;
			VertBuffDesc.Size = static_cast<uint32_t>(sizeof(typename M::vertexBufferType)) * vertSize;

			Diligent::BufferData VBData;
			VBData.pData = this->_mesh->vertices.data();
			VBData.DataSize = VertBuffDesc.Size;

			device->CreateBuffer(VertBuffDesc, &VBData, &this->_vbh);
			if (this->_vbh == nullptr) RAWRBOX_CRITICAL("Failed to create vertex buffer");

			rawrbox::BarrierUtils::barrier({{this->_vbh, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		}

	public:
		ModelBase(size_t vertices = 0, size_t indices = 0) {
			this->_mesh = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(vertices, indices);
			this->_material = std::make_unique<M>();
		};

		ModelBase(ModelBase&&) = delete;
		ModelBase& operator=(ModelBase&&) = delete;
		ModelBase(const ModelBase&) = delete;
		ModelBase& operator=(const ModelBase&) = delete;
		virtual ~ModelBase() {
			RAWRBOX_DESTROY(this->_visibleBuffer);
			RAWRBOX_DESTROY(this->_visibility);

			RAWRBOX_DESTROY(this->_argsBuffer);

			RAWRBOX_DESTROY(this->_vbh);
			RAWRBOX_DESTROY(this->_ibh);

			this->_mesh.reset();
		}

		// BLEND SHAPES ---
		bool createBlendShape(const std::string& id, const std::vector<rawrbox::Vector3f>& newVertexPos, const std::vector<rawrbox::Vector4f>& newNormPos, float weight = 0.F) {
			if (this->_mesh == nullptr) RAWRBOX_CRITICAL("Mesh not initialized!");

			auto blend = std::make_unique<rawrbox::BlendShapes<M>>();
			blend->pos = newVertexPos;
			blend->normals = newNormPos;
			blend->weight = weight;
			blend->mesh = this->_mesh.get();

			this->_blend_shapes[id] = std::move(blend);
			return true;
		}

		virtual bool removeBlendShape(const std::string& id) {
			auto fnd = this->_blend_shapes.find(id);
			if (fnd == this->_blend_shapes.end()) return false;

			this->_blend_shapes.erase(id);
			this->applyBlendShapes();
			return true;
		}

		virtual bool setBlendShape(const std::string& id, float weight) {
			auto fnd = this->_blend_shapes.find(id);
			if (fnd == this->_blend_shapes.end()) return false;

			fnd->second->weight = weight;
			this->applyBlendShapes();
			return true;
		}

		virtual bool setBlendShapeByKey(const std::string& id, float weight) {
			bool found = false;

			for (auto& s : this->_blend_shapes) {
				auto split = rawrbox::StrUtils::split(s.first, '-');
				auto shapeId = split[split.size() - 1];
				if (shapeId == id) {
					s.second->weight = weight;
					found = true;
				}
			}

			if (found) this->applyBlendShapes();
			return found;
		}
		// --------------

		// UTIL ---
		// Partial update ---
		virtual void updateVertexRange(size_t firstVertex, size_t count) {
			if (!this->isDynamic() || !this->isUploaded()) RAWRBOX_CRITICAL("Model is not dynamic or not uploaded");
			const auto stride = sizeof(typename M::vertexBufferType);

			this->_dirtyVB.push(firstVertex * stride, count * stride);
			this->_requiresUpdate = true;
			this->_vertexBoundsDirty = true;
		}

		virtual void updateIndexRange(size_t firstIndex, size_t count) {
			if (!this->isDynamic() || !this->isUploaded()) RAWRBOX_CRITICAL("Model is not dynamic or not uploaded");

			this->_dirtyIB.push(firstIndex * sizeof(uint32_t), count * sizeof(uint32_t));
			this->_requiresUpdate = true;
		}

		virtual void updateBuffers() {
			this->_requiresUpdate = this->isDynamic() && this->isUploaded();
			this->_vertexBoundsDirty = true;

			this->_dirtyVB.markFull();
			this->_dirtyIB.markFull();
		}

		[[nodiscard]] virtual uint32_t getID(int /*index*/ = -1) const { return this->_mesh->getID(); }
		virtual void setID(uint32_t id, int /*index*/ = -1) {
			this->_mesh->setID(id);
		}

		[[nodiscard]] virtual const rawrbox::Color& getColor(int /*index*/ = -1) const { return this->_mesh->getColor(); }
		virtual void setColor(const rawrbox::Color& color, int /*index*/ = -1) {
			this->_mesh->setColor(color);
		}

		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const { return this->_mesh->getPos(); }
		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->_mesh->setPos(pos);
		}

		[[nodiscard]] virtual const rawrbox::Vector3f& getScale() const { return this->_mesh->getScale(); }
		virtual void setScale(const rawrbox::Vector3f& scale) {
			this->_mesh->setScale(scale);
		}

		[[nodiscard]] virtual const rawrbox::Vector4f& getAngle() const { return this->_mesh->getAngle(); }
		virtual void setAngle(const rawrbox::Vector4f& ang) {
			this->_mesh->setAngle(ang);
		}
		virtual void setEulerAngle(const rawrbox::Vector3f& ang) {
			this->_mesh->setEulerAngle(ang);
		}

		[[nodiscard]] virtual const rawrbox::Matrix4x4& getMatrix() const {
			return this->_mesh->matrix;
		}

		[[nodiscard]] virtual bool isDynamic() const {
			return this->_uploadType != rawrbox::UploadType::STATIC;
		}
		[[nodiscard]] virtual bool isUploaded() const {
			return (this->_vbh != nullptr && this->_ibh != nullptr);
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* mesh() {
			return this->_mesh.get();
		}

		// ----
		virtual void upload(rawrbox::UploadType type = rawrbox::UploadType::STATIC) {
			if (this->isUploaded()) RAWRBOX_CRITICAL("Already uploaded!");
			this->_uploadType = type;

			// BUFFERS ----
			this->createVertexBuffer();
			this->createIndexBuffer();
			// ---------------------

			// Initialize material ----
			this->_material->init();
			// ------------
		}

		virtual void draw() {
			if (!this->isUploaded()) RAWRBOX_CRITICAL("Failed to render model, vertex / index buffer is not uploaded");
			if (this->_material == nullptr) RAWRBOX_CRITICAL("Material not set");

			this->bindBuffers();

			// Reset material uniforms ----
			this->_material->resetUniformBinds();
			// ----------------------------
		}

		void drawLate() override {}

		// SHADOWS ---
		[[nodiscard]] bool canCastShadows() const override {
			return this->isUploaded() && this->_material != nullptr && this->_material->shadow != nullptr;
		}

		[[nodiscard]] const rawrbox::BBOX& getCachedVertexBounds() {
			if (this->_vertexBoundsDirty) {
				this->_vertexBounds = getVertexBounds(*this->_mesh);
				this->_vertexBoundsDirty = false;
			}

			return this->_vertexBounds;
		}

		[[nodiscard]] rawrbox::BBOX getShadowBounds() override {
			return this->getCachedVertexBounds().transform(this->getMatrix());
		}

		[[nodiscard]] bool canCastShadows(const rawrbox::Mesh<typename M::vertexBufferType>& mesh) const {
			return this->canCastShadows() && mesh.castShadows() && !mesh.isTransparent() && !mesh.getWireframe() && !mesh.getLineMode();
		}

		void bindShadowMesh(const rawrbox::Mesh<typename M::vertexBufferType>& mesh, const rawrbox::Matrix4x4& matrix) {
			if (this->_material == nullptr) RAWRBOX_CRITICAL("Material not set");

			if (this->_material->shadow == nullptr) RAWRBOX_CRITICAL("Shadow not initialized!");
			rawrbox::RENDERER->context()->SetPipelineState(this->_material->shadow);

			this->_material->bindVertexUniforms(mesh);
			this->_material->bindVertexSkinnedUniforms(mesh);
			this->_material->bindPixelUniforms(mesh);

			rawrbox::RENDERER->getPassCamera()->setModelTransform(matrix);
		}

		void drawShadow(const rawrbox::ShadowDraw& draw) override {
			if (this->_material == nullptr) RAWRBOX_CRITICAL("Material not set");
			if (!this->canCastShadows(*this->_mesh)) return;

			if (draw.bind) {
				this->bindBuffers();
				this->_material->resetUniformBinds();
				this->bindShadowMesh(*this->_mesh, this->getMatrix());
			}

			const rawrbox::ShadowSubmit shadowBind = {
			    .bounds = this->getShadowBounds(),
			    .indices = this->_mesh->totalIndex,

			    .firstIndex = this->_mesh->baseIndex,
			    .baseVertex = this->_mesh->baseVertex,

			    .instances = 1U,
			    .view = rawrbox::ShadowDraw::ALL_VIEWS};

			draw.submit(shadowBind);
		}
		// -----------
	};
} // namespace rawrbox