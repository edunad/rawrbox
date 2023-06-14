#pragma once

#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/model/light/directional.hpp>
#include <rawrbox/render/model/light/point.hpp>
#include <rawrbox/render/model/light/spot.hpp>
#include <rawrbox/render/model/material/instanced.hpp>
#include <rawrbox/render/model/model.hpp>

namespace rawrbox {

	struct Instance {
		rawrbox::Matrix4x4 matrix;
		rawrbox::Colorf color;
	};

	template <typename M = rawrbox::MaterialInstancedUnlit>
	class InstancedModel : public rawrbox::ModelBase<M> {
		std::vector<Instance> _instances = {};
		rawrbox::TextureBase* _texture = nullptr;

	public:
		InstancedModel() = default;
		InstancedModel(const InstancedModel&) = delete;
		InstancedModel(InstancedModel&&) = delete;
		InstancedModel& operator=(const InstancedModel&) = delete;
		InstancedModel& operator=(InstancedModel&&) = delete;
		~InstancedModel() override {
			this->_instances.clear();
			this->_texture = nullptr;
		}

		virtual void setMesh(rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			auto& v = mesh.getVertices();
			auto& i = mesh.getIndices();

			if (v.empty() || i.empty()) throw std::runtime_error("[RawrBox-InstancedModel] Invalid mesh! Missing vertices / indices!");

			this->_vertices = mesh.vertices;
			this->_indices = mesh.indices;
			this->_texture = mesh.texture;
		}

		virtual void addInstance(const rawrbox::Instance& instance) {
			this->_instances.push_back(instance);
		}

		[[nodiscard]] rawrbox::Instance& getMesh(size_t i = 0) const {
			if (i < 0 || i >= this->_instances.size()) throw std::runtime_error("[RawrBox-InstancedModel] Failed to find instance");
			return this->_instances[i];
		}

		void draw() override {
			if ((BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported) == 0) throw std::runtime_error("[RawrBox-InstancedModel] Instancing not supported by the graphics card!");
			ModelBase<M>::draw();

			// Setup instance
			uint32_t instanceStride = sizeof(rawrbox::Instance);
			uint32_t instances = bgfx::getAvailInstanceDataBuffer(this->_instances.size(), instanceStride);
			if (instances <= 0) return;

			bgfx::InstanceDataBuffer idb = {};
			bgfx::allocInstanceDataBuffer(&idb, instances, instanceStride);

			auto data = std::bit_cast<rawrbox::Instance*>(idb.data);
			for (uint32_t ii = 0; ii < instances; ++ii) {
				std::memcpy(data, &this->_instances[ii], instanceStride);
				// data += instanceStride;
			}

			// ----
			// this->_material->process(this->_texture->getHandle());

			if (this->isDynamicBuffer()) {
				bgfx::setVertexBuffer(0, this->_vbdh);
				bgfx::setIndexBuffer(this->_ibdh);
			} else {
				bgfx::setVertexBuffer(0, this->_vbh);
				bgfx::setIndexBuffer(this->_ibh);
			}

			// Set instance data buffer.
			bgfx::setInstanceDataBuffer(&idb, 0, instances);
			bgfx::setState(BGFX_STATE_DEFAULT, 0);

			this->_material->postProcess();
		}
	};
} // namespace rawrbox
