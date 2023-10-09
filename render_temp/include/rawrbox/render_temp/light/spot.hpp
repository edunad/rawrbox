#pragma once
#include <rawrbox/render_temp/light/base.hpp>

namespace rawrbox {
	class SpotLight : public rawrbox::LightBase {
	protected:
		float _innerCone;
		float _outerCone;

	public:
		SpotLight(const rawrbox::Vector3f& posMatrix, const rawrbox::Vector3f& direction, const rawrbox::Colorf& color, float innerCone, float outerCone, float power);
		SpotLight(const SpotLight&) = delete;
		SpotLight(SpotLight&&) = delete;
		SpotLight& operator=(const SpotLight&) = delete;
		SpotLight& operator=(SpotLight&&) = delete;
		~SpotLight() override = default;

		[[nodiscard]] rawrbox::LightType getType() const override;
		[[nodiscard]] const rawrbox::Vector4f getData() const override;

		[[nodiscard]] float getInnerCone() const;
		[[nodiscard]] float getOuterCone() const;
	};
} // namespace rawrbox
