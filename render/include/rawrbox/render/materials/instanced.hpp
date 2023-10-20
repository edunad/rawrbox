#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	class MaterialInstanced : public rawrbox::MaterialBase {
		static std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> _pipelines;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		static Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _SRB;

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
