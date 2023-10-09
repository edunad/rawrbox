#pragma once
#include <rawrbox/render_temp/materials/lit.hpp>

namespace rawrbox {

	class MaterialSkinnedLit : public rawrbox::MaterialLit {
	protected:
		void setupUniforms() override;

	public:
		MaterialSkinnedLit() = default;
		MaterialSkinnedLit(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit& operator=(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit(const MaterialSkinnedLit&) = delete;
		MaterialSkinnedLit& operator=(const MaterialSkinnedLit&) = delete;
		~MaterialSkinnedLit() override = default;

		void upload() override;

		[[nodiscard]] uint32_t supports() const override;
		[[nodiscard]] const bgfx::VertexLayout vLayout() const override;
	};
} // namespace rawrbox
