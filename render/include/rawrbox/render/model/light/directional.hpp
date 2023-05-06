#pragma once
#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
		#include <rawrbox/debug/gizmos.hpp>
	#endif
#endif

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/base.hpp>

namespace rawrbox {
	class LightDirectional : public rawrbox::LightBase {
	protected:
		rawrbox::Vector3 _direction;

	public:
		LightDirectional(rawrbox::Vector3f posMatrix, rawrbox::Vector3 dir, rawrbox::Colorf diffuse, rawrbox::Colorf specular) : rawrbox::LightBase(posMatrix, diffuse, specular), _direction(dir) {
#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
			GIZMOS::get().addLight(this);
	#endif
#endif
		};

		LightDirectional(LightDirectional&&) = delete;
		LightDirectional& operator=(LightDirectional&&) = delete;
		LightDirectional(const LightDirectional&) = delete;
		LightDirectional& operator=(const LightDirectional&) = delete;
		~LightDirectional() override {
#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
			GIZMOS::get().removeLight(this);
	#endif
#endif
		};

		LightType getType() override { return LightType::LIGHT_DIR; };
		std::array<float, 16> const getDataMatrix() override {
			return {
			    this->_diffuse.r, this->_specular.r, this->_direction.x, static_cast<float>(this->getType()),
			    this->_diffuse.g, this->_specular.g, this->_direction.y, 0,
			    this->_diffuse.b, this->_specular.b, this->_direction.z, 0,
			    0, 0, 0, this->_isOn ? 1.F : 0};
		}
	};
} // namespace rawrbox
