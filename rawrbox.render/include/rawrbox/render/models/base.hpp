#pragma once

#include <rawrbox/render/materials/unlit.hpp>
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

	struct ModelOriginalData {
		rawrbox::Vector3f pos = {};
		uint32_t normal = {};
	};

	template <typename M = rawrbox::MaterialUnlit>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	class ModelBase {

	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _vbh; // Vertices
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _ibh; // Indices

		std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>> _mesh = nullptr;
		std::unique_ptr<M> _material = nullptr;

		std::unordered_map<std::string, std::unique_ptr<rawrbox::BlendShapes<M>>> _blend_shapes = {};
		std::vector<rawrbox::ModelOriginalData> _original_data = {};

		// DYNAMIC SUPPORT ---
		rawrbox::UploadType _uploadType = rawrbox::UploadType::STATIC;
		bool _requiresUpdate = false;
		// ----

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Model");
		// -------------

		// BLEND SHAPES ---
		virtual void applyBlendShapes() {
			// Reset vertex ---------
			for (auto& shape : this->_blend_shapes) {
				if (!shape.second->isActive() || shape.second->mesh == nullptr) continue;

				auto& verts = shape.second->mesh->vertices;
				for (size_t i = 0; i < verts.size(); i++) {
					auto& data = this->_original_data[i];
					verts[i].position = data.pos;

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						verts[i].normal = data.normal;
					}
				}
			}
			// --------

			for (auto& shape : this->_blend_shapes) {
				if (!shape.second->isActive() || shape.second->mesh == nullptr) continue;

				auto& verts = shape.second->mesh->vertices;

				auto& blendPos = shape.second->pos;
				auto& blendNormals = shape.second->normals;

				if (!blendPos.empty() && blendPos.size() != verts.size()) {
					this->_logger->info("Blendshape verts do not match with the mesh '{}' verts! Total verts: {}, blend shape verts: {}", shape.first, verts.size(), blendPos.size());
					return;
				}

				if (!blendNormals.empty() && blendNormals.size() != verts.size()) {
					this->_logger->info("Blendshape normals do not match with the mesh '{}' verts! Total verts: {}, blend shape verts: {}", shape.first, verts.size(), blendNormals.size());
					return;
				}

				float step = std::clamp(shape.second->weight, 0.F, 1.F);

				// Apply vertices ----
				for (size_t i = 0; i < blendPos.size(); i++) {
					verts[i].position = verts[i].position.lerp(blendPos[i], step);
				}
				// -------------------

				// Apply normal ----
				if constexpr (supportsNormals<typename M::vertexBufferType>) {
					for (size_t i = 0; i < blendNormals.size(); i++) {
						rawrbox::Vector4f unpacked = rawrbox::Vector4f(rawrbox::PackUtils::fromNormal(verts[i].normal)).lerp(blendNormals[i], step); // meh
						verts[i].normal = rawrbox::PackUtils::packNormal(unpacked.x, unpacked.y, unpacked.z);
					}
				}
				// -------------------
			}
		}
		// --------------

		virtual void internalUpdate() {
			if (!this->isUploaded() || !this->isDynamic() || !this->_requiresUpdate) return;
			auto* context = rawrbox::RENDERER->context();

			auto vertSize = static_cast<uint64_t>(this->_mesh->vertices.size());
			auto indcSize = static_cast<uint64_t>(this->_mesh->indices.size());

			auto empty = vertSize <= 0 || indcSize <= 0;

			uint64_t sizeVB = sizeof(typename M::vertexBufferType) * vertSize;
			uint64_t sizeIB = sizeof(uint16_t) * indcSize;

			bool resizable = this->_uploadType == rawrbox::UploadType::RESIZABLE_DYNAMIC;

			bool resizeVertex = resizable && sizeVB > this->_vbh->GetDesc().Size;
			bool resizeIndex = resizable && sizeIB > this->_ibh->GetDesc().Size;

			if (resizeVertex) {
				RAWRBOX_DESTROY(this->_vbh);

				this->_mesh->vertices.reserve(vertSize + this->BUFFER_INCREASE_OFFSET);
				this->createVertexBuffer();

				this->_logger->info("Resizing vertex buffer ({} -> {})", vertSize, this->_mesh->vertices.capacity());
			}

			if (resizeIndex) {
				RAWRBOX_DESTROY(this->_ibh);

				this->_mesh->indices.reserve(indcSize + this->BUFFER_INCREASE_OFFSET);
				this->createIndexBuffer();

				this->_logger->info("Resizing index buffer ({} -> {})", indcSize, this->_mesh->indices.capacity());
			}

			if (resizeVertex && resizeIndex) {
				this->_requiresUpdate = false;
				return;
			}

			// BARRIER -----
			std::vector<Diligent::StateTransitionDesc> barriers = {};
			if (!resizeVertex) barriers.emplace_back(this->_vbh, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			if (!resizeIndex) barriers.emplace_back(this->_ibh, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

			rawrbox::BarrierUtils::barrier(barriers);

			if (!resizeVertex) context->UpdateBuffer(this->_vbh, 0, vertSize * sizeof(typename M::vertexBufferType), empty ? nullptr : this->_mesh->vertices.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			if (!resizeIndex) context->UpdateBuffer(this->_ibh, 0, indcSize * sizeof(uint16_t), empty ? nullptr : this->_mesh->indices.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

			barriers.clear();
			if (!resizeVertex) barriers.emplace_back(this->_vbh, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			if (!resizeIndex) barriers.emplace_back(this->_ibh, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

			rawrbox::BarrierUtils::barrier(barriers);
			// -----------

			this->_requiresUpdate = false;
		}

		virtual void createIndexBuffer() {
			auto* device = rawrbox::RENDERER->device();

			auto indcSize = static_cast<uint64_t>(std::max<size_t>(this->_mesh->indices.capacity(), this->isDynamic() ? 1U : 0U)); // RESIZABLE requires at least one vertice
			if (indcSize <= 0) throw this->_logger->error("Indices data cannot be empty!");

			Diligent::BufferDesc IndcBuffDesc;
			IndcBuffDesc.Name = "RawrBox::Buffer::Indices";
			IndcBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			IndcBuffDesc.Usage = this->isDynamic() ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE; // TODO: Diligent::USAGE_SPARSE;
			IndcBuffDesc.Size = static_cast<uint32_t>(sizeof(uint16_t)) * indcSize;

			Diligent::BufferData IBData;
			IBData.pData = this->_mesh->indices.data();
			IBData.DataSize = IndcBuffDesc.Size;

			device->CreateBuffer(IndcBuffDesc, &IBData, &this->_ibh);
			if (this->_ibh == nullptr) throw this->_logger->error("Failed to create index buffer");

			rawrbox::BarrierUtils::barrier({{this->_ibh, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_INDEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		}

		virtual void createVertexBuffer() {
			auto* device = rawrbox::RENDERER->device();

			auto vertSize = static_cast<uint64_t>(std::max<size_t>(this->_mesh->vertices.capacity(), this->isDynamic() ? 1U : 0U)); // RESIZABLE requires at least one vertice
			if (vertSize <= 0) throw this->_logger->error("Vertices data cannot be empty!");

			Diligent::BufferDesc VertBuffDesc;
			VertBuffDesc.Name = "RawrBox::Buffer::Vertex";
			VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			VertBuffDesc.Usage = this->isDynamic() ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE; // TODO:  Diligent::USAGE_SPARSE;
			VertBuffDesc.Size = static_cast<uint32_t>(sizeof(typename M::vertexBufferType)) * vertSize;

			Diligent::BufferData VBData;
			VBData.pData = this->_mesh->vertices.data();
			VBData.DataSize = VertBuffDesc.Size;

			device->CreateBuffer(VertBuffDesc, &VBData, &this->_vbh);
			if (this->_vbh == nullptr) throw this->_logger->error("Failed to create vertex buffer");

			rawrbox::BarrierUtils::barrier({{this->_vbh, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_VERTEX_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		}

	public:
		// To prevent the vertex / index buffer from resizing too often, increase this value to offset the scaling based on your model needs
		uint64_t BUFFER_INCREASE_OFFSET = 256;

		ModelBase(size_t vertices = 0, size_t indices = 0) {
			this->_mesh = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(vertices, indices);
			this->_material = std::make_unique<M>();
		};

		ModelBase(ModelBase&&) = delete;
		ModelBase& operator=(ModelBase&&) = delete;
		ModelBase(const ModelBase&) = delete;
		ModelBase& operator=(const ModelBase&) = delete;
		virtual ~ModelBase() {
			RAWRBOX_DESTROY(this->_vbh);
			RAWRBOX_DESTROY(this->_ibh);

			this->_mesh.reset();
		}

		// BLEND SHAPES ---
		bool createBlendShape(const std::string& id, const std::vector<rawrbox::Vector3f>& newVertexPos, const std::vector<rawrbox::Vector4f>& newNormPos, float weight = 0.F) {
			if (this->_mesh == nullptr) throw this->_logger->error("Mesh not initialized!");

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

			if (found) {
				this->applyBlendShapes();
			}

			return found;
		}
		// --------------

		// UTIL ---
		virtual void updateBuffers() {
			if (!this->isDynamic() || !this->isUploaded()) throw this->_logger->error("Model is not dynamic or uploaded!");

			auto vertSize = static_cast<uint32_t>(this->_mesh->vertices.size());
			auto indcSize = static_cast<uint32_t>(this->_mesh->indices.size());
			auto empty = vertSize <= 0 || indcSize <= 0;

			// Store original positions for blendstates
			if (!empty && _original_data.size() != vertSize) {
				_original_data.clear();
				_original_data.reserve(vertSize);

				for (auto& v : this->_mesh->vertices) {
					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						_original_data.push_back({v.position, v.normal});
					} else {
						_original_data.push_back({v.position, {}});
					}
				}
			}
			// -----------

			this->_requiresUpdate = true;
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
			if (this->isUploaded()) throw this->_logger->error("Already uploaded!");
			this->_uploadType = type;

			// Store original positions for blendstates
			if (this->isDynamic() && this->_mesh->vertices.empty()) {
				_original_data.reserve(this->_mesh->vertices.capacity());

				for (auto& v : this->_mesh->vertices) {
					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						_original_data.push_back({v.position, v.normal});
					} else {
						_original_data.push_back({v.position, 0x00000000});
					}
				}
			}
			// -----------

			// BUFFERS ----
			this->createVertexBuffer();
			this->createIndexBuffer();
			// ---------------------

			// Initialize material ----
			this->_material->init();
			// ------------
		}

		virtual void draw() {
			if (!this->isUploaded()) throw this->_logger->error("Failed to render model, vertex / index buffer is not uploaded");
			if (this->_material == nullptr) throw this->_logger->error("Material not set");

			auto* context = rawrbox::RENDERER->context();

			// Execute pending buffer updates --
			this->internalUpdate();
			// --------------------------

			// Bind vertex and index buffers
			std::array<Diligent::IBuffer*, 1> pBuffs = {this->_vbh};

			context->SetVertexBuffers(0, 1, pBuffs.data(), nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(this->_ibh, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			// ----

			// Reset material uniforms ----
			this->_material->resetUniformBinds();
			// ----------------------------
		}
	};
} // namespace rawrbox
