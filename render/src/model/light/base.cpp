#include <rawrbox/render/model/light/manager.hpp>

namespace rawrbox {
	LightBase::LightBase(rawrbox::Vector3f pos, rawrbox::Colorf diffuse, rawrbox::Colorf specular) : _pos(pos), _diffuse(diffuse), _specular(specular){};

	const rawrbox::Colorf& LightBase::getSpecularColor() const { return this->_specular; }
	const rawrbox::Colorf& LightBase::getDiffuseColor() const { return this->_diffuse; }
	const rawrbox::Colorf& LightBase::getAmbientColor() const { return this->_ambient; }

	const float LightBase::getConstant() const { return this->_constant; }
	const float LightBase::getLinear() const { return this->_linear; }
	const float LightBase::getQuadratic() const { return this->_quadratic; }

	void LightBase::setId(size_t id) { this->_id = id; };
	const size_t LightBase::id() const { return this->_id; };

	const bool LightBase::isOn() const { return this->_isOn; }
	void LightBase::setStatus(bool on) { this->_isOn = on; };

	const rawrbox::Vector3f& LightBase::getPos() const { return this->_pos; }
	void LightBase::setPos(const rawrbox::Vector3f& pos) { this->_pos = pos; }

	const rawrbox::Vector3f& LightBase::getOffsetPos() const { return this->_offset; }
	void LightBase::setOffsetPos(const rawrbox::Vector3f& pos) { this->_offset = pos; }

	const std::array<float, 4> LightBase::getPosMatrix() const {
		auto p = this->getPos() + this->getOffsetPos();
		return {p.x, p.y, p.z, 0};
	}

	const rawrbox::Matrix4x4 LightBase::getDataMatrix() const { return {}; }
	const rawrbox::LightType LightBase::getType() const { return rawrbox::LightType::LIGHT_UNKNOWN; }

} // namespace rawrbox
