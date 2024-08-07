#pragma once
#include <rawrbox/render/lights/base.hpp>

namespace rawrbox {
	class DirectionalLight : public rawrbox::LightBase {
	protected:
	public:
		DirectionalLight(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colorf& color);
		DirectionalLight(const DirectionalLight&) = delete;
		DirectionalLight(DirectionalLight&&) = delete;
		DirectionalLight& operator=(const DirectionalLight&) = delete;
		DirectionalLight& operator=(DirectionalLight&&) = delete;
		~DirectionalLight() override = default;

		[[nodiscard]] rawrbox::LightType getType() const override;
	};
} // namespace rawrbox
