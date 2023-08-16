#pragma once

#include <rawrbox/render/light/base.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class LightBaseWrapper {
	protected:
		std::weak_ptr<rawrbox::LightBase> _ref;

	public:
		LightBaseWrapper(const std::shared_ptr<rawrbox::LightBase>& ref);
		LightBaseWrapper(const LightBaseWrapper&) = default;
		LightBaseWrapper(LightBaseWrapper&&) = default;
		LightBaseWrapper& operator=(const LightBaseWrapper&) = default;
		LightBaseWrapper& operator=(LightBaseWrapper&&) = default;
		virtual ~LightBaseWrapper();

		// UTILS ----
		[[nodiscard]] virtual const rawrbox::Colori getColor() const;
		virtual void setColor(const rawrbox::Colori& col);

		[[nodiscard]] virtual const rawrbox::Vector4f getData() const;

		virtual void setRadius(float radius);
		[[nodiscard]] virtual float getRadius() const;

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
		// ------

		[[nodiscard]] virtual bool isValid() const;
		[[nodiscard]] virtual rawrbox::LightBase* getRef() const;

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
