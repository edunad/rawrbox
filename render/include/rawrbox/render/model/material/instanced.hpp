#pragma once

#include <rawrbox/render/model/material/base.hpp>

namespace rawrbox {

	class MaterialInstancedUnlit : public rawrbox::MaterialBase {
	public:
		MaterialInstancedUnlit() = default;
		MaterialInstancedUnlit(MaterialInstancedUnlit&&) = delete;
		MaterialInstancedUnlit& operator=(MaterialInstancedUnlit&&) = delete;
		MaterialInstancedUnlit(const MaterialInstancedUnlit&) = delete;
		MaterialInstancedUnlit& operator=(const MaterialInstancedUnlit&) = delete;
		~MaterialInstancedUnlit() override = default;

		void upload() override;
	};

} // namespace rawrbox
