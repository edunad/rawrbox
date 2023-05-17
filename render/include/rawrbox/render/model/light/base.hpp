#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/model/light/types.hpp>

#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
		#include <rawrbox/debug/gizmos.hpp>
	#endif
#endif

#include <fmt/format.h>

#include <array>

namespace rawrbox {
	// class LIGHTS;
	class LightBase {
	protected:
		bool _isOn = true;
		size_t _id = 0;

		rawrbox::Vector3f _posMatrix;
		rawrbox::Vector3f _offsetPos;
		rawrbox::Vector3f _prevPosMatrix;

		rawrbox::Colorf _diffuse = rawrbox::Colors::White;
		rawrbox::Colorf _specular = rawrbox::Colors::White;
		rawrbox::Colorf _ambient = rawrbox::Colors::White;

		void updateGizmo() {
#ifndef RAWRBOX_TESTING
	#ifdef RAWRBOX_DEBUG
			rawrbox::GIZMOS::updateGizmo(fmt::format("Light-{}", this->id()), this->getPos() + this->_offsetPos);
	#endif
#endif
		}

	public:
		LightBase() = default;
		LightBase(rawrbox::Vector3f posMatrix, rawrbox::Colorf diffuse, rawrbox::Colorf specular) : _posMatrix(posMatrix), _diffuse(diffuse), _specular(specular){};
		virtual ~LightBase() {
			rawrbox::LIGHTS::removeLight(this);
		};

		[[nodiscard]] virtual const rawrbox::Colorf& getSpecularColor() const { return this->_specular; }
		[[nodiscard]] virtual const rawrbox::Colorf& getDiffuseColor() const { return this->_diffuse; }
		[[nodiscard]] virtual const rawrbox::Colorf& getAmbientColor() const { return this->_ambient; }

		virtual void setId(size_t id) { this->_id = id; };
		[[nodiscard]] virtual const size_t id() const { return this->_id; };

		[[nodiscard]] virtual const bool isOn() const { return this->_isOn; }
		virtual void setStatus(bool on) { this->_isOn = on; };

		[[nodiscard]] const rawrbox::Vector3f& getPos() const { return this->_posMatrix; }
		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->_posMatrix = pos;
			this->updateGizmo();
		}

		[[nodiscard]] const rawrbox::Vector3f& getOffsetPos() const { return this->_offsetPos; }
		virtual void setOffsetPos(const rawrbox::Vector3f& pos) {
			this->_offsetPos = pos;
			this->updateGizmo();
		}

		virtual std::array<float, 4> getPosMatrix() {
			auto p = this->getPos() + this->getOffsetPos();
			return {p.x, p.y, p.z, 0};
		}

		virtual rawrbox::Matrix4x4 const getDataMatrix() = 0;
		virtual rawrbox::LightType getType() = 0;
	};
} // namespace rawrbox
