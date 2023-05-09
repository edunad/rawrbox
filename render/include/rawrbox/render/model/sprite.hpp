#pragma once
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/material/base.hpp>

#define BGFX_STATE_DEFAULT_SPRITE (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)
namespace rawrbox {

	template <typename M = rawrbox::MaterialBase>
	class Sprite : public rawrbox::ModelBase<M> {
	public:
		using ModelBase<M>::ModelBase;

		bool supportsOptimization() override { return false; }
		void addMesh(std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> mesh) override {
			mesh->addData("billboard_mode", {1.F, 0, 0}); // Force billboard for sprites
			ModelBase<M>::addMesh(mesh);
		}

		void draw(const rawrbox::Vector3f& camPos) override {
			ModelBase<M>::draw(camPos);

			for (auto& mesh : this->_meshes) {
				this->_material->process(mesh);

				bgfx::setTransform((this->_matrix * mesh->offsetMatrix).data());

				if (this->isDynamicBuffer()) {
					bgfx::setVertexBuffer(0, this->_vbdh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibdh, mesh->baseIndex, mesh->totalIndex);
				} else {
					bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);
				}

				uint64_t flags = BGFX_STATE_DEFAULT_SPRITE | mesh->culling | mesh->blending;
				if (mesh->wireframe) flags |= BGFX_STATE_PT_LINES;

				bgfx::setState(flags, 0);

				this->_material->postProcess();
			}
		}
	};
} // namespace rawrbox
