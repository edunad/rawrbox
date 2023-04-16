#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>

#include <array>

namespace rawrBox {
	class LightBase {
	protected:
		bool _isOn = true;

		rawrBox::Vector3f _pos;
		rawrBox::Colorf _color = rawrBox::Colors::White;

	public:
		uint32_t indx;

		virtual void setStatus(bool on) { this->_isOn = on; };
		virtual void setPosition(rawrBox::Vector3f p) { this->_pos = p; };
		virtual void setColor(rawrBox::Colorf p) { this->_color = p; };
		virtual rawrBox::Colorf getColor() { return this->_color; };

		LightBase(rawrBox::Vector3f pos, rawrBox::Colorf color) : _pos(pos), _color(color){};
		virtual ~LightBase() = default;

		virtual std::array<float, 16> getMatrix() = 0;
	};
} // namespace rawrBox
