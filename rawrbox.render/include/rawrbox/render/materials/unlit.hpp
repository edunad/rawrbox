#pragma once

#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {
	class MaterialUnlit : public rawrbox::MaterialBase {
		static bool _built;

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialUnlit() = default;
		MaterialUnlit(const MaterialUnlit&) = delete;
		MaterialUnlit(MaterialUnlit&&) = delete;
		MaterialUnlit& operator=(const MaterialUnlit&) = delete;
		MaterialUnlit& operator=(MaterialUnlit&&) = delete;
		~MaterialUnlit() override = default;

		void init() override;
		void createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, const Diligent::ShaderMacroHelper& helper = {}) override;
	};
} // namespace rawrbox
