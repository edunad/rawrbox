#pragma once

#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/plugins/clustered.hpp>

namespace rawrbox {

	class MaterialLit : public rawrbox::MaterialBase {
		static bool _built;

	public:
		using vertexBufferType = rawrbox::VertexNormData;

		MaterialLit() = default;
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() override = default;

		void init() override;
		void createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, Diligent::ShaderMacroHelper helper = {}) override;
	};
} // namespace rawrbox
