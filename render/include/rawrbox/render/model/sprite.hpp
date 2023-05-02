#pragma once
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/material/base.hpp>

#define BGFX_STATE_DEFAULT_SPRITE (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)
namespace rawrBox {

	template <typename M = rawrBox::MaterialBase>
	class Sprite : public rawrBox::ModelBase<M> {
	public:
		using ModelBase<M>::ModelBase;

		bool supportsOptimization() override { return false; }
		void addMesh(std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> mesh) override {
			mesh->addData("billboard_mode", {1.F, 0, 0}); // Force billboard for sprites
			this->_meshes.push_back(std::move(mesh));
		}

		void draw(const rawrBox::Vector3f& camPos) override {
			ModelBase<M>::draw(camPos);

			for (auto& mesh : this->_meshes) {
				this->_material->process(mesh);

				bgfx::setTransform(this->_matrix.data());

				bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
				bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);

				uint64_t flags = BGFX_STATE_DEFAULT_SPRITE | mesh->culling | mesh->blending;
				if (mesh->wireframe) flags |= BGFX_STATE_PT_LINES;

				bgfx::setState(flags, 0);

				this->_material->postProcess();
			}
		}
	};
} // namespace rawrBox
