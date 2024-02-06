#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/lights/types.hpp>

#include <fmt/format.h>

#ifdef RAWRBOX_SCRIPTING
//	#include <sol/sol.hpp>
#endif

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
		float _intensity = 1.F;

#ifdef RAWRBOX_SCRIPTING
		// sol::object _luaWrapper;
		// virtual void initializeLua();
#endif

	public:
		LightBase(const rawrbox::Vector3f& pos, const rawrbox::Colorf& color, float radius);
		LightBase(const LightBase&) = delete;
		LightBase(LightBase&&) = delete;
		LightBase& operator=(const LightBase&) = delete;
		LightBase& operator=(LightBase&&) = delete;

#ifdef RAWRBOX_SCRIPTING
		virtual ~LightBase();
#else
		virtual ~LightBase() = default;
#endif

		virtual void setColor(const rawrbox::Colorf& col);
		[[nodiscard]] virtual const rawrbox::Colorf getColor() const;

		[[nodiscard]] virtual const rawrbox::Vector4f getData() const;

		[[nodiscard]] virtual float getRadius() const;
		virtual void setRadius(float radius);

		[[nodiscard]] virtual float getIntensity() const;
		virtual void setIntensity(float intensity);

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

#ifdef RAWRBOX_SCRIPTING
		// virtual sol::object& getScriptingWrapper();
#endif
	};
} // namespace rawrbox
