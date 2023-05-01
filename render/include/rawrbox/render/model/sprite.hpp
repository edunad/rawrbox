#pragma once
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/material/sprite_unlit.hpp>

#define BGFX_STATE_DEFAULT_SPRITE (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)
namespace rawrBox {

	template <typename M = rawrBox::MaterialSpriteUnlit>
	class Sprite : public rawrBox::ModelBase<M> {
	public:
		using ModelBase<M>::ModelBase;

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
