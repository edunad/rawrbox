#pragma once

#include <rawrbox/render/materials/lit.hpp>

namespace rawrbox {

	class MaterialInstancedLit : public rawrbox::MaterialLit {
	public:
		MaterialInstancedLit() = default;
		MaterialInstancedLit(MaterialInstancedLit&&) = delete;
		MaterialInstancedLit& operator=(MaterialInstancedLit&&) = delete;
		MaterialInstancedLit(const MaterialInstancedLit&) = delete;
		MaterialInstancedLit& operator=(const MaterialInstancedLit&) = delete;
		~MaterialInstancedLit() override = default;

		void upload() override;
	};

} // namespace rawrbox
