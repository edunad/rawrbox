#pragma once
#include <rawrbox/render/light/base.hpp>

namespace rawrbox {
	class PointLight : public rawrbox::LightBase {
	protected:
	public:
		using rawrbox::LightBase::LightBase;

		PointLight(const PointLight&) = default;
		PointLight(PointLight&&) = delete;
		PointLight& operator=(const PointLight&) = default;
		PointLight& operator=(PointLight&&) = delete;
		~PointLight() override = default;

		[[nodiscard]] const rawrbox::LightType getType() const override { return LightType::POINT; };
	};
} // namespace rawrbox
