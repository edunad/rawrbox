#pragma once
#include <rawrbox/render/model/material/base.hpp>

namespace rawrbox {
	class MaterialParticle : public rawrbox::MaterialBase {
	public:
		MaterialParticle() = default;
		MaterialParticle(const MaterialParticle &) = delete;
		MaterialParticle(MaterialParticle &&) = delete;
		MaterialParticle &operator=(const MaterialParticle &) = delete;
		MaterialParticle &operator=(MaterialParticle &&) = delete;
		~MaterialParticle() override = default;

		void upload() override;
	};
} // namespace rawrbox
