#pragma once

#include <rawrbox/render_temp/materials/base.hpp>

namespace rawrbox {
	class MaterialInstanced : public rawrbox::MaterialBase {
	public:
		MaterialInstanced() = default;
		MaterialInstanced(MaterialInstanced&&) = delete;
		MaterialInstanced& operator=(MaterialInstanced&&) = delete;
		MaterialInstanced(const MaterialInstanced&) = delete;
		MaterialInstanced& operator=(const MaterialInstanced&) = delete;
		~MaterialInstanced() override = default;

		[[nodiscard]] uint32_t supports() const override;
		void upload() override;
	};

} // namespace rawrbox
