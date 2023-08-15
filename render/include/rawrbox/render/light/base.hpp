#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/light/types.hpp>

#include <fmt/format.h>

namespace rawrbox {

#ifdef RAWRBOX_SCRIPTING
	class LightBase : public std::enable_shared_from_this<rawrbox::LightBase> {
#else
	class LightBase {
#endif
	protected:
		bool _isOn = true;
		size_t _id = 0;

		rawrbox::Vector3f _pos = {};
		rawrbox::Vector3f _offset = {};
		rawrbox::Vector3f _direction = {};

		rawrbox::Colorf _color = rawrbox::Colors::White();

		float _radius = 5.F;

	public:
		LightBase(const rawrbox::Vector3f& pos, const rawrbox::Colorf& color, float radius);
		LightBase(const LightBase&) = delete;
		LightBase(LightBase&&) = delete;
		LightBase& operator=(const LightBase&) = delete;
		LightBase& operator=(LightBase&&) = delete;
		virtual ~LightBase() = default;

		[[nodiscard]] virtual const rawrbox::Colorf getColor() const;
		virtual void setColor(const rawrbox::Colorf& col);

		[[nodiscard]] virtual const rawrbox::Vector4f getData() const;

		virtual void setRadius(float radius);
		[[nodiscard]] virtual float getRadius() const;

		virtual void setId(size_t id);
		[[nodiscard]] virtual size_t id() const;

		[[nodiscard]] virtual bool isOn() const;
		virtual void setStatus(bool on);

		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const;
		virtual void setPos(const rawrbox::Vector3f& pos);

		[[nodiscard]] virtual const rawrbox::Vector3f& getOffsetPos() const;
		virtual void setOffsetPos(const rawrbox::Vector3f& pos);

		[[nodiscard]] virtual const rawrbox::Vector3f getWorldPos() const;
		[[nodiscard]] virtual rawrbox::LightType getType() const;

		[[nodiscard]] virtual const rawrbox::Vector3f& getDirection() const;
		virtual void setDirection(const rawrbox::Vector3f& dir);
	};
} // namespace rawrbox
