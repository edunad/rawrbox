#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	class MaterialInstanced : public rawrbox::MaterialBase {
	public:
		MaterialInstanced() = default;
		MaterialInstanced(MaterialInstanced&&) = delete;
		MaterialInstanced& operator=(MaterialInstanced&&) = delete;
		MaterialInstanced(const MaterialInstanced&) = delete;
		MaterialInstanced& operator=(const MaterialInstanced&) = delete;
		~MaterialInstanced() override = default;

		void upload() override;
	};

} // namespace rawrbox
