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
			}
			// -----------
		}
	};
} // namespace rawrbox
