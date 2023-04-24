#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/model.h>
#include <rawrbox/render/static.h>

#include <bx/math.h>

#include <cstdint>

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA_TO_COVERAGE)

namespace rawrBox {
	void Model::draw(const rawrBox::Vector3f& camPos) {
		ModelBase::draw(camPos);

		for (auto& mesh : this->_meshes) {
			this->_material->process(mesh);

			bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
			bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);

			float matrix[16];
			bx::mtxMul(matrix, mesh->offsetMatrix.data(), this->_matrix.data());
			bgfx::setTransform(matrix);

			uint64_t flags = BGFX_STATE_DEFAULT_3D | mesh->culling | mesh->blending;
			if (mesh->wireframe) flags |= BGFX_STATE_PT_LINES;

			bgfx::setState(flags, 0);
			this->_material->postProcess();
		}
	}
} // namespace rawrBox
