#pragma once

#include <rawrbox/render/materials/instanced_lit.hpp>

namespace rawrbox {

	class MaterialDecal : public rawrbox::MaterialInstancedLit {
	protected:
		void setupUniforms() override;

	public:
		MaterialDecal() = default;
		MaterialDecal(MaterialDecal&&) = delete;
		MaterialDecal& operator=(MaterialDecal&&) = delete;
		MaterialDecal(const MaterialDecal&) = delete;
		MaterialDecal& operator=(const MaterialDecal&) = delete;
		~MaterialDecal() override = default;

		void process(const rawrbox::Mesh& mesh) override;
		void upload() override;
	};

} // namespace rawrbox
