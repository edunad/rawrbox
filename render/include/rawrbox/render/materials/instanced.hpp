#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	class MaterialInstanced : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

		static Diligent::IPipelineState* _base;
		static Diligent::IPipelineState* _cullback;
		static Diligent::IPipelineState* _wireframe;
		static Diligent::IPipelineState* _cullnone;

		static Diligent::IShaderResourceBinding* _bind;

		void bindPipeline(const rawrbox::Mesh& mesh) override;

	public:
		MaterialInstanced() = default;
		MaterialInstanced(MaterialInstanced&&) = delete;
		MaterialInstanced& operator=(MaterialInstanced&&) = delete;
		MaterialInstanced(const MaterialInstanced&) = delete;
		MaterialInstanced& operator=(const MaterialInstanced&) = delete;
		~MaterialInstanced() override = default;

		static void init();

		[[nodiscard]] uint32_t supports() const override;
		[[nodiscard]] static const std::pair<std::vector<Diligent::LayoutElement>, uint32_t> vLayout();
	};

} // namespace rawrbox
