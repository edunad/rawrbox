#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {
	class MaterialText3D : public rawrbox::MaterialBase {
	public:
		MaterialText3D() = default;
		void upload() override;
	};

} // namespace rawrbox
