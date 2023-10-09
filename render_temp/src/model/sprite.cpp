#include <rawrbox/render_temp/model/sprite.hpp>

#define BGFX_STATE_DEFAULT_SPRITE (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)

namespace rawrbox {
	bool Sprite::xAxisEnabled() const { return this->_xAxis; }
	void Sprite::lockXAxix(bool locked) { this->_xAxis = !locked; }
	bool Sprite::yAxisEnabled() const { return this->_yAxis; }
	void Sprite::lockYAxix(bool locked) { this->_yAxis = !locked; }
	bool Sprite::zAxisEnabled() const { return this->_zAxis; }
	void Sprite::lockZAxix(bool locked) { this->_zAxis = !locked; }

	rawrbox::Mesh* Sprite::addMesh(rawrbox::Mesh mesh) {
		mesh.setOptimizable(false);
		return Model::addMesh(mesh);
	}

	void Sprite::draw() {
		rawrbox::ModelBase::draw();

		for (auto& mesh : this->_meshes) {
			// Set billboard ----
			mesh->addData("billboard_mode", {this->_xAxis ? 1.F : 0.F, this->_yAxis ? 1.F : 0.F, this->_zAxis ? 1.F : 0.F, 0.F}); // Force billboard for sprites
			// -----

			this->_material->process(*mesh);

			if (this->isDynamic()) {
				bgfx::setVertexBuffer(0, this->_vbdh, mesh->baseVertex, mesh->totalVertex);
				bgfx::setIndexBuffer(this->_ibdh, mesh->baseIndex, mesh->totalIndex);
			} else {
				bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
				bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);
			}

			bgfx::setTransform((this->getMatrix() * mesh->getMatrix()).data());

			uint64_t flags = BGFX_STATE_DEFAULT_SPRITE | mesh->culling | mesh->blending | mesh->depthTest;
			flags |= mesh->lineMode ? BGFX_STATE_PT_LINES : mesh->wireframe ? BGFX_STATE_PT_LINESTRIP
											: 0;

			bgfx::setState(flags, 0);
			this->_material->postProcess();
		}
	}
} // namespace rawrbox
