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

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {

			auto context = rawrbox::RENDERER->context();

			// SETUP VERTEX UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::BindlessVertexBuffer> CBConstants(context, rawrbox::BindlessManager::signatureBufferVertex, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				this->bindBaseUniforms<T>(mesh, CBConstants);
			}
			// -----------

			// SETUP PIXEL UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::BindlessPixelBuffer> CBConstants(context, rawrbox::BindlessManager::signatureBufferPixel, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				this->bindBasePixelUniforms<T>(mesh, CBConstants);

				CBConstants->litData = mesh.textures.getData();
			}
			// -----------
		}
	};
} // namespace rawrbox
