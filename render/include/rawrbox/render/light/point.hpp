#pragma once
#include <rawrbox/render/light/base.hpp>

namespace rawrbox {
	class LightPoint : public rawrbox::LightBase {
	protected:
	public:
		using rawrbox::LightBase::LightBase;

		LightPoint(const LightPoint&) = default;
		LightPoint(LightPoint&&) = delete;
		LightPoint& operator=(const LightPoint&) = default;
		LightPoint& operator=(LightPoint&&) = delete;
		~LightPoint() override = default;

		[[nodiscard]] const rawrbox::LightType getType() const override { return LightType::LIGHT_POINT; };
	};
} // namespace rawrbox
