#pragma once
#include <rawrbox/render_temp/light/base.hpp>

namespace rawrbox {
	class PointLight : public rawrbox::LightBase {
	protected:
	public:
		using rawrbox::LightBase::LightBase;

		PointLight(const PointLight&) = delete;
		PointLight(PointLight&&) = delete;
		PointLight& operator=(const PointLight&) = delete;
		PointLight& operator=(PointLight&&) = delete;
		~PointLight() override = default;

		[[nodiscard]] rawrbox::LightType getType() const override;
	};
} // namespace rawrbox
