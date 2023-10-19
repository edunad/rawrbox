#include <rawrbox/render/models/sprite.hpp>

namespace rawrbox {

	// TODO: UPDATE MESHES DATA
	bool Sprite::xAxisEnabled() const { return this->_xAxis; }
	void Sprite::lockXAxix(bool locked) { this->_xAxis = !locked; }
	bool Sprite::yAxisEnabled() const { return this->_yAxis; }
	void Sprite::lockYAxix(bool locked) { this->_yAxis = !locked; }
	bool Sprite::zAxisEnabled() const { return this->_zAxis; }
	void Sprite::lockZAxix(bool locked) { this->_zAxis = !locked; }
	// -----

	rawrbox::Mesh* Sprite::addMesh(rawrbox::Mesh mesh) {
		mesh.setOptimizable(false);
		mesh.addData("billboard_mode", {this->_xAxis ? 1.F : 0.F, this->_yAxis ? 1.F : 0.F, this->_zAxis ? 1.F : 0.F, 0.F}); // Force billboard for sprites

		return Model::addMesh(mesh);
	}
} // namespace rawrbox
