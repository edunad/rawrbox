#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/mesh.hpp>

#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
		#include <rawrbox/debug/static.hpp>
	#endif
#endif

#include <array>

namespace rawrbox {

	enum LightType {
		LIGHT_UNKNOWN = 0,

		LIGHT_POINT,
		LIGHT_SPOT,
		LIGHT_DIR,
	};

	class LightBase {
	protected:
		bool _isOn = true;
		size_t _id = 0;

#ifndef RAWRBOX_TESTING
		std::weak_ptr<rawrbox::Mesh<>> _parentMesh;
#endif
		rawrbox::Vector3f _posMatrix;
		rawrbox::Vector3f _prevPosMatrix;

		rawrbox::Colorf _diffuse = rawrbox::Colors::White;
		rawrbox::Colorf _specular = rawrbox::Colors::White;
		rawrbox::Colorf _ambient = rawrbox::Colors::White;

	public:
		LightBase() = default;
		LightBase(rawrbox::Vector3f posMatrix, rawrbox::Colorf diffuse, rawrbox::Colorf specular) : _posMatrix(posMatrix), _diffuse(diffuse), _specular(specular){};
		virtual ~LightBase() {
#ifndef RAWRBOX_TESTING
			this->_parentMesh.reset();
#endif
		};

		[[nodiscard]] virtual const rawrbox::Colorf& getSpecularColor() const { return this->_specular; }
		[[nodiscard]] virtual const rawrbox::Colorf& getDiffuseColor() const { return this->_diffuse; }
		[[nodiscard]] virtual const rawrbox::Colorf& getAmbientColor() const { return this->_ambient; }

		virtual void setId(size_t id) { this->_id = id; };
		[[nodiscard]] virtual const size_t id() const { return this->_id; };

		[[nodiscard]] virtual const bool isOn() const { return this->_isOn; }
		virtual void setStatus(bool on) { this->_isOn = on; };

#ifndef RAWRBOX_TESTING
		void setParent(std::shared_ptr<rawrbox::Mesh<>> parent) {
			this->_parentMesh = parent;
		}
#endif

		virtual void setPos(rawrbox::Vector3f pos) {
			this->_posMatrix = pos;
		}

		virtual std::array<float, 4> getPosMatrix() {
			std::array<float, 4> pos = {this->_posMatrix.x, this->_posMatrix.y, this->_posMatrix.z, 0};

#ifndef RAWRBOX_TESTING
			if (!this->_parentMesh.expired()) {
				auto p = this->_parentMesh.lock()->offsetMatrix;

				pos[0] += p[12];
				pos[1] += p[13];
				pos[2] += p[14];
			}

	#ifdef RAWRBOX_DEBUG
			rawrbox::GIZMOS.updateGizmo(fmt::format("Light-{}", this->id()), {pos[0], pos[1], pos[2]});
	#endif
#endif
			return pos;
		}

		virtual rawrbox::Matrix4x4 const getDataMatrix() = 0;
		virtual LightType getType() = 0;
	};
} // namespace rawrbox
