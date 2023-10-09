#pragma once

#include <rawrbox/render_temp/materials/base.hpp>

namespace rawrbox {
	class MaterialText3D : public rawrbox::MaterialBase {
	public:
		MaterialText3D() = default;
		MaterialText3D(const MaterialText3D &) = delete;
		MaterialText3D(MaterialText3D &&) = delete;
		MaterialText3D &operator=(const MaterialText3D &) = delete;
		MaterialText3D &operator=(MaterialText3D &&) = delete;
		~MaterialText3D() override = default;

		[[nodiscard]] uint32_t supports() const override;
		void upload() override;
	};

} // namespace rawrbox
