#pragma once
#include <rawrbox/render_temp/materials/base.hpp>

namespace rawrbox {

	class MaterialSkinned : public rawrbox::MaterialBase {
	protected:
		void setupUniforms() override;

	public:
		MaterialSkinned() = default;
		MaterialSkinned(MaterialSkinned&&) = delete;
		MaterialSkinned& operator=(MaterialSkinned&&) = delete;
		MaterialSkinned(const MaterialSkinned&) = delete;
		MaterialSkinned& operator=(const MaterialSkinned&) = delete;
		~MaterialSkinned() override = default;

		void upload() override;

		[[nodiscard]] uint32_t supports() const override;
		[[nodiscard]] const bgfx::VertexLayout vLayout() const override;
	};
} // namespace rawrbox
