#pragma once
#include <rawrbox/render_temp/materials/base.hpp>

namespace rawrbox {
	class MaterialParticle : public rawrbox::MaterialBase {
	public:
		MaterialParticle() = default;
		MaterialParticle(const MaterialParticle &) = delete;
		MaterialParticle(MaterialParticle &&) = delete;
		MaterialParticle &operator=(const MaterialParticle &) = delete;
		MaterialParticle &operator=(MaterialParticle &&) = delete;
		~MaterialParticle() override = default;

		[[nodiscard]] uint32_t supports() const override;
		void upload() override;
	};
} // namespace rawrbox
