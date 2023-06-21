#pragma once
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/model.hpp>

#define BGFX_STATE_DEFAULT_SPRITE (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)

namespace rawrbox {

	template <typename M = rawrbox::MaterialBase>
	class Sprite : public rawrbox::Model<M> {
	protected:
		bool _xAxis = true;
		bool _yAxis = true;
		bool _zAxis = true;

	public:
		Sprite() = default;
		Sprite(const Sprite&) = delete;
		Sprite(Sprite&&) = delete;
		Sprite& operator=(const Sprite&) = delete;
		Sprite& operator=(Sprite&&) = delete;
		~Sprite() override = default;

		[[nodiscard]] const bool xAxisEnabled() const { return this->_xAxis; }
		void lockXAxix(bool locked) { this->_xAxis = !locked; }
		[[nodiscard]] const bool yAxisEnabled() const { return this->_yAxis; }
		void lockYAxix(bool locked) { this->_yAxis = !locked; }
		[[nodiscard]] const bool zAxisEnabled() const { return this->_zAxis; }
		void lockZAxix(bool locked) { this->_zAxis = !locked; }

		void addMesh(rawrbox::Mesh mesh) override {
			mesh.setOptimizable(false);
			Model<M>::addMesh(mesh);
		}

		void draw() override {
			ModelBase<M>::draw();

			for (auto& mesh : this->_meshes) {
				// Set billboard ----
				mesh->addData("billboard_mode", {this->_xAxis ? 1.F : 0.F, this->_yAxis ? 1.F : 0.F, this->_zAxis ? 1.F : 0.F, 0.F}); // Force billboard for sprites
				// -----

				this->_material->process(*mesh);

				if (this->isDynamicBuffer()) {
					bgfx::setVertexBuffer(0, this->_vbdh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibdh, mesh->baseIndex, mesh->totalIndex);
				} else {
					bgfx::setVertexBuffer(0, this->_vbh, mesh->baseVertex, mesh->totalVertex);
					bgfx::setIndexBuffer(this->_ibh, mesh->baseIndex, mesh->totalIndex);
				}

				bgfx::setTransform((this->getMatrix() * mesh->matrix).data());

				uint64_t flags = BGFX_STATE_DEFAULT_SPRITE | mesh->culling | mesh->blending | mesh->depthTest;
				flags |= mesh->lineMode ? BGFX_STATE_PT_LINES : mesh->wireframe ? BGFX_STATE_PT_LINESTRIP
												: 0;

				bgfx::setState(flags, 0);
				this->_material->postProcess();
				bgfx::discard();
			}
		}
	};
} // namespace rawrbox
