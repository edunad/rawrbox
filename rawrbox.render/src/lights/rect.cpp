#include <rawrbox/math/utils/pack.hpp>
#include <rawrbox/render/lights/rect.hpp>
#include <rawrbox/render/static.hpp>

#include <cmath>

namespace rawrbox {
	RectLight::RectLight(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Vector3f& right, const rawrbox::Colorf& color, const rawrbox::Vector2f& size, float radius) : rawrbox::LightBase(pos, color, radius), _right(right), _size(size) {
		this->_direction = direction.normalized();
		this->orthonormalizeRight();
	}

	void RectLight::orthonormalizeRight() {
		auto right = this->_right - this->_direction * this->_right.dot(this->_direction);
		if (right.length() <= 1e-4F)
			right = (std::abs(this->_direction.y) < 0.99F ? rawrbox::Vector3f{0.F, 1.F, 0.F} : rawrbox::Vector3f{1.F, 0.F, 0.F}).cross(this->_direction);

		this->_right = right.normalized();
	}

	rawrbox::LightType RectLight::getType() const { return LightType::RECT; }

	rawrbox::Vector4f RectLight::getData() const {
		const auto oct = rawrbox::PackUtils::encodeOctahedron(this->_right.x, this->_right.y, this->_right.z);
		return {this->_size.x * 0.5F, this->_size.y * 0.5F, oct[0], oct[1]};
	}

	void RectLight::setDirection(const rawrbox::Vector3f& dir) {
		rawrbox::LightBase::setDirection(dir);
		this->orthonormalizeRight();
	}

	const rawrbox::Vector3f& RectLight::getRight() const { return this->_right; }
	void RectLight::setRight(const rawrbox::Vector3f& right) {
		this->_right = right;
		this->orthonormalizeRight();

		rawrbox::__LIGHT_DIRTY__ = true;
	}

	const rawrbox::Vector2f& RectLight::getSize() const { return this->_size; }
	void RectLight::setSize(const rawrbox::Vector2f& size) {
		if (this->_size == size) return;

		this->_size = size;
		rawrbox::__LIGHT_DIRTY__ = true;
	}
} // namespace rawrbox