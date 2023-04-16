#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>

#include <rawrbox/render/model/light/base.hpp>

namespace rawrBox {
	class LightPoint : public rawrBox::LightBase {
	protected:
		float _fallOffStart = 1.0f;
		float _fallOffEnd = 10.0f;
		float _luminance = 1.0f;

	public:
		LightPoint(rawrBox::Vector3f pos, rawrBox::Colorf color, float fallOffStart, float fallOffEnd, float luminance) : rawrBox::LightBase(pos, color), _fallOffStart(fallOffStart), _fallOffEnd(fallOffEnd), _luminance(std::clamp(luminance, 0.f, 1.f)){};
		virtual std::array<float, 16> getMatrix() override {
			return {this->_pos.x, this->_color.r, this->_fallOffStart, 0,
			    this->_pos.y, this->_color.g, this->_fallOffStart, 0,
			    this->_pos.z, this->_color.b, this->_luminance, 0,
			    0, 0.f, this->_isOn ? 1.f : 0, 0};
		}
	};
} // namespace rawrBox
