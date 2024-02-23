#pragma once

#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {
	class MaterialInstanced : public rawrbox::MaterialUnlit {
		static bool _built;

	public:
		MaterialInstanced() = default;
		MaterialInstanced(MaterialInstanced&&) = delete;
		MaterialInstanced& operator=(MaterialInstanced&&) = delete;
		MaterialInstanced(const MaterialInstanced&) = delete;
		MaterialInstanced& operator=(const MaterialInstanced&) = delete;
		~MaterialInstanced() override = default;

		void init() override;
	};

} // namespace rawrbox
