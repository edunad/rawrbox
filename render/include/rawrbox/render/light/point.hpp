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

		[[nodiscard]] const float getRadius() const override {
			// radius = where attenuation would lead to an intensity of 1W/m^2
			const float INTENSITY_CUTOFF = 1.0F;
			const float ATTENTUATION_CUTOFF = 0.05F;
			auto intensity = this->getFlux() / (4.0F * rawrbox::pi<float>);

			float maxIntensity = intensity.max();
			float attenuation = std::max(INTENSITY_CUTOFF, ATTENTUATION_CUTOFF * maxIntensity) / maxIntensity;
			return 1.0F / sqrtf(attenuation);
		};
	};
} // namespace rawrbox
