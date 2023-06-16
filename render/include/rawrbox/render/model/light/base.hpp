#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/types.hpp>
#include <rawrbox/render/model/mesh.hpp>

#include <fmt/format.h>

#include <array>

namespace rawrbox {

	class LightBase {
	protected:
		bool _isOn = true;
		size_t _id = 0;

		rawrbox::Vector3f _pos = {};
		rawrbox::Vector3f _offset = {};

		rawrbox::Colorf _diffuse = rawrbox::Colors::White;
		rawrbox::Colorf _specular = rawrbox::Colors::White;
		rawrbox::Colorf _ambient = rawrbox::Colors::White;

	public:
		LightBase(rawrbox::Vector3f posMatrix, rawrbox::Colorf diffuse, rawrbox::Colorf specular);
		LightBase(const LightBase&) = default;
		LightBase(LightBase&&) = default;

		LightBase& operator=(const LightBase&) = default;
		LightBase& operator=(LightBase&&) = delete;
		virtual ~LightBase();

		[[nodiscard]] virtual const rawrbox::Colorf& getSpecularColor() const;
		[[nodiscard]] virtual const rawrbox::Colorf& getDiffuseColor() const;
		[[nodiscard]] virtual const rawrbox::Colorf& getAmbientColor() const;

		virtual void setId(size_t id);
		[[nodiscard]] virtual const size_t id() const;

		[[nodiscard]] virtual const bool isOn() const;
		virtual void setStatus(bool on);

		[[nodiscard]] const rawrbox::Vector3f& getPos() const;
		virtual void setPos(const rawrbox::Vector3f& pos);

		[[nodiscard]] const rawrbox::Vector3f& getOffsetPos() const;
		virtual void setOffsetPos(const rawrbox::Vector3f& pos);

		[[nodiscard]] virtual const std::array<float, 4> getPosMatrix() const;
		[[nodiscard]] virtual const rawrbox::Matrix4x4 getDataMatrix() const;
		[[nodiscard]] virtual const rawrbox::LightType getType() const;
	};
} // namespace rawrbox
