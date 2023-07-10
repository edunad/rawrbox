#include <rawrbox/render/light/manager.hpp>

namespace rawrbox {
	LightBase::LightBase(const rawrbox::Vector3f& pos, const rawrbox::Colorf& color, float intensity) : _pos(pos), _color(color), _intensity(intensity){};

	const rawrbox::Colorf LightBase::getFlux() const { return this->_color * this->_intensity; }
	const float LightBase::getRadius() const { return 0; }

	void LightBase::setId(size_t id) { this->_id = id; };
	const size_t LightBase::id() const { return this->_id; };

	const bool LightBase::isOn() const { return this->_isOn; }
	void LightBase::setStatus(bool on) { this->_isOn = on; };

	const rawrbox::Vector3f& LightBase::getPos() const { return this->_pos; }
	void LightBase::setPos(const rawrbox::Vector3f& pos) { this->_pos = pos; }

	const rawrbox::Vector3f& LightBase::getOffsetPos() const { return this->_offset; }
	void LightBase::setOffsetPos(const rawrbox::Vector3f& pos) { this->_offset = pos; }

	const rawrbox::Vector3f LightBase::getWorldPos() const { return this->getPos() + this->getOffsetPos(); }
	const rawrbox::LightType LightBase::getType() const { return rawrbox::LightType::LIGHT_UNKNOWN; }

} // namespace rawrbox
