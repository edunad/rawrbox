#pragma once

#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/utils/string.hpp>

#include <Graphics/GraphicsEngine/interface/Buffer.h>

#ifdef RAWRBOX_SCRIPTING
	#include <sol/sol.hpp>
#endif

#include <typeinfo>

namespace rawrbox {

	template <typename M = rawrbox::MaterialBase>
	struct BlendShapes {
	public:
		float weight = 0.F;

		rawrbox::Mesh<typename M::vertexBufferType>* mesh = nullptr; // For quick access

		std::vector<rawrbox::Vector3f> pos = {};
		std::vector<rawrbox::Vector3f> normals = {};

		bool isActive() { return weight > 0.F; }
		BlendShapes() = default;
	};

#ifdef RAWRBOX_SCRIPTING
	class ModelBase : public std::enable_shared_from_this<rawrbox::ModelBase> {
#else
	template <typename M = rawrbox::MaterialBase>
	class ModelBase {
#endif

	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _vbh; // Vertices
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _ibh; // Indices

		std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>> _mesh = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>();
		std::unique_ptr<M> _material = std::make_unique<M>();

		std::unordered_map<std::string, std::unique_ptr<rawrbox::BlendShapes<M>>> _blend_shapes = {};

		// BGFX DYNAMIC SUPPORT ---
		bool _isDynamic = false;
		// ----

#ifdef RAWRBOX_SCRIPTING
		sol::object _luaWrapper;
		virtual void initializeLua() {
			if (!SCRIPTING::initialized) return;
			this->_luaWrapper = sol::make_object(rawrbox::SCRIPTING::getLUA(), rawrbox::ModelBaseWrapper(this->shared_from_this()));
		}
#endif

		// BLEND SHAPES ---
		virtual void applyBlendShapes() {
			// Reset vertex ---------
			for (auto& shape : this->_blend_shapes) {
				if (!shape.second->isActive() || shape.second->mesh == nullptr) continue;

				auto& verts = shape.second->mesh->vertices;
				for (auto& v : verts) {
					// v.reset(); // TODO::
				}
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
				if constexpr (supportsNormals<typename M::vertexBufferType>) {
					for (size_t i = 0; i < blendNormals.size(); i++) {
						verts[i].normal = verts[i].normal.lerp(blendNormals[i], step);
					}
				}
				// -------------------
			}
		}
		// --------------

	public:
		ModelBase() = default;
		ModelBase(ModelBase&&) = delete;
		ModelBase& operator=(ModelBase&&) = delete;
		ModelBase(const ModelBase&) = delete;
		ModelBase& operator=(const ModelBase&) = delete;
		virtual ~ModelBase() {
			RAWRBOX_DESTROY(this->_vbh);
			RAWRBOX_DESTROY(this->_ibh);

			this->_mesh.reset();

#ifdef RAWRBOX_SCRIPTING
			if (this->_luaWrapper.valid()) this->_luaWrapper.abandon();
#endif
		}

		// BLEND SHAPES ---
		bool createBlendShape(const std::string& id, const std::vector<rawrbox::Vector3f>& newVertexPos, const std::vector<rawrbox::Vector3f>& newNormPos, float weight = 0.F) {
			if (this->_mesh == nullptr) throw std::runtime_error("[RawrBox-ModelBase] Mesh not initialized!");

			auto blend = std::make_unique<rawrbox::BlendShapes<M>>();
			blend->pos = newVertexPos;
			blend->normals = newNormPos;
			blend->weight = weight;
			blend->mesh = this->_mesh.get();

			this->_blend_shapes[id] = std::move(blend);
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
			if (!this->isDynamic() || !this->isUploaded()) return;

			auto context = rawrbox::RENDERER->context();

			auto vertSize = static_cast<uint32_t>(this->_mesh->vertices.size());
			auto indcSize = static_cast<uint32_t>(this->_mesh->indices.size());
			auto empty = vertSize <= 0 || indcSize <= 0;

			// ----------------------------------------

			context->UpdateBuffer(this->_vbh, 0, vertSize * sizeof(typename M::vertexBufferType), empty ? nullptr : this->_mesh->vertices.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context->UpdateBuffer(this->_ibh, 0, indcSize * sizeof(uint16_t), empty ? nullptr : this->_mesh->indices.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
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
			return this->_isDynamic;
		}
		[[nodiscard]] virtual bool isUploaded() const {
			return this->_vbh != nullptr && this->_ibh != nullptr;
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* mesh() {
			return this->_mesh.get();
		}

		// ----
		virtual void upload(bool dynamic = false) {
			if (this->isUploaded()) throw std::runtime_error("[RawrBox-ModelBase] Upload called twice");

			auto device = rawrbox::RENDERER->device();
			auto context = rawrbox::RENDERER->context();

			// Generate buffers ----
			this->_isDynamic = dynamic;

			auto vertSize = static_cast<uint32_t>(this->_mesh->vertices.size());
			auto indcSize = static_cast<uint32_t>(this->_mesh->indices.size());

			if (!dynamic && vertSize <= 0) throw std::runtime_error("[RawrBox-ModelBase] Vertices cannot be empty on non-dynamic buffer!");
			if (!dynamic && indcSize <= 0) throw std::runtime_error("[RawrBox-ModelBase] Indices cannot be empty on non-dynamic buffer!");

			// VERT ----
			Diligent::BufferDesc VertBuffDesc;
			VertBuffDesc.Name = "RawrBox::Buffer::Vertex";
			VertBuffDesc.Usage = dynamic ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE;
			VertBuffDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			VertBuffDesc.Size = vertSize * static_cast<uint32_t>(sizeof(typename M::vertexBufferType));

			Diligent::BufferData VBData;
			VBData.pData = this->_mesh->vertices.data();
			VBData.DataSize = VertBuffDesc.Size;
			device->CreateBuffer(VertBuffDesc, vertSize > 0 ? &VBData : nullptr, &this->_vbh);
			// ---------------------

			// INDC ----
			Diligent::BufferDesc IndcBuffDesc;
			IndcBuffDesc.Name = "RawrBox::Buffer::Indices";
			IndcBuffDesc.Usage = dynamic ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE;
			IndcBuffDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			IndcBuffDesc.Size = indcSize * sizeof(uint16_t);

			Diligent::BufferData IBData;
			IBData.pData = this->_mesh->indices.data();
			IBData.DataSize = IndcBuffDesc.Size;
			device->CreateBuffer(IndcBuffDesc, indcSize > 0 ? &IBData : nullptr, &this->_ibh);
			// ---------------------
		}

		virtual void draw() {
			if (!this->isUploaded()) throw std::runtime_error("[RawrBox-Model] Failed to render model, vertex / index buffer is not uploaded");

			// Bind vertex and index buffers
			const uint64_t offset = 0;
			// NOLINTBEGIN(*)
			Diligent::IBuffer* pBuffs[] = {this->_vbh};
			// NOLINTEND(*)

			auto context = rawrbox::RENDERER->context();
			context->SetVertexBuffers(0, 1, pBuffs, &offset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(this->_ibh, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			// ----
		}

#ifdef RAWRBOX_SCRIPTING
		virtual sol::object& getScriptingWrapper() {
			if (!this->_luaWrapper.valid()) this->initializeLua();
			return this->_luaWrapper;
		}
#endif
	};
} // namespace rawrbox
