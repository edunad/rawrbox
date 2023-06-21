#pragma once

#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/model/light/directional.hpp>
#include <rawrbox/render/model/light/point.hpp>
#include <rawrbox/render/model/light/spot.hpp>
#include <rawrbox/render/model/material/instanced.hpp>
#include <rawrbox/render/model/model.hpp>

namespace rawrbox {

	struct Instance {
		rawrbox::Matrix4x4 matrix = {};
		rawrbox::Colorf color = rawrbox::Colors::White;
		rawrbox::Vector4f extraData = {}; // AtlasID, etc..

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float) // Position
			    .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord2, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord3, 4, bgfx::AttribType::Float)

			    .add(bgfx::Attrib::TexCoord4, 4, bgfx::AttribType::Float) // Color
			    .add(bgfx::Attrib::TexCoord5, 4, bgfx::AttribType::Float) // ExtraData
			    .end();
			return l;
		};
	};

	template <typename M = rawrbox::MaterialInstancedUnlit>
	class InstancedModel : public rawrbox::ModelBase<M> {
		bgfx::DynamicVertexBufferHandle _dataBuffer = BGFX_INVALID_HANDLE;
		std::vector<rawrbox::Instance> _instances = {};

		void updateBuffers() override {
			rawrbox::ModelBase<M>::updateBuffers();
			this->updateInstance();
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
			RAWRBOX_DESTROY(this->_dataBuffer);
			this->_instances.clear();
		}

		virtual void setTemplate(rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			if (mesh.empty()) throw std::runtime_error("[RawrBox-InstancedModel] Invalid mesh! Missing vertices / indices!");
			this->_mesh = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh);

			if (this->isUploaded() && this->isDynamicBuffer()) {
				this->updateBuffers();
			}
		}

		virtual void addInstance(const rawrbox::Instance& instance) {
			this->_instances.push_back(instance);
			if (this->isUploaded()) this->updateInstance();
		}

		virtual void removeInstance(size_t i = 0) {
			if (i < 0 || i >= this->_instances.size()) throw std::runtime_error("[RawrBox-InstancedModel] Failed to find instance");
			this->_instances.erase(this->_instances.begin() + i);

			if (this->isUploaded()) this->updateInstance();
		}

		[[nodiscard]] const rawrbox::Instance& getInstance(size_t i = 0) const {
			if (i < 0 || i >= this->_instances.size()) throw std::runtime_error("[RawrBox-InstancedModel] Failed to find instance");
			return this->_instances[i];
		}

		std::vector<rawrbox::Instance>& instances() { return this->_instances; }

		void upload(bool dynamic = false) override {
			rawrbox::ModelBase<M>::upload(dynamic);

			this->_dataBuffer = bgfx::createDynamicVertexBuffer(
			    1, rawrbox::Instance::vLayout(), BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);

			this->updateInstance();
		}

		void updateInstance() {
			if (!bgfx::isValid(this->_dataBuffer)) throw std::runtime_error("[RawrBox-InstancedModel] Data buffer not valid! Did you call upload()?");

			const bgfx::Memory* mem = bgfx::makeRef(this->_instances.data(), static_cast<uint32_t>(this->_instances.size()) * rawrbox::Instance::vLayout().m_stride);
			bgfx::update(this->_dataBuffer, 0, mem);
		}

		void draw() override {
			if (this->_instances.empty()) return;
			if ((BGFX_CAPS_INSTANCING & bgfx::getCaps()->supported) == 0) throw std::runtime_error("[RawrBox-InstancedModel] Instancing not supported by the graphics card!");

			ModelBase<M>::draw();
			this->_material->process(*this->_mesh); // Set atlas

			if (this->isDynamicBuffer()) {
				bgfx::setVertexBuffer(0, this->_vbdh);
				bgfx::setIndexBuffer(this->_ibdh);
			} else {
				bgfx::setVertexBuffer(0, this->_vbh);
				bgfx::setIndexBuffer(this->_ibh);
			}

			// Set instance data buffer.
			bgfx::setTransform((this->getMatrix()).data());
			bgfx::setBuffer(6, this->_dataBuffer, bgfx::Access::Read);
			bgfx::setInstanceDataBuffer(this->_dataBuffer, 0, this->_instances.size());
			// ----

			uint64_t flags = BGFX_STATE_DEFAULT_3D | this->_mesh->culling | this->_mesh->blending | this->_mesh->depthTest;
			flags |= this->_mesh->lineMode ? BGFX_STATE_PT_LINES : this->_mesh->wireframe ? BGFX_STATE_PT_LINESTRIP
												      : 0;

			bgfx::setState(flags, 0);
			this->_material->postProcess();
			bgfx::discard();
		}
	};
} // namespace rawrbox
