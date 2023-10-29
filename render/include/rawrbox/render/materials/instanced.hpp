#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	class MaterialInstanced : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

	public:
		using vertexBufferType = rawrbox::VertexData;

	protected:
		void prepareMaterial() override;

	public:
		MaterialInstanced() = default;
		MaterialInstanced(MaterialInstanced&&) = delete;
		MaterialInstanced& operator=(MaterialInstanced&&) = delete;
		MaterialInstanced(const MaterialInstanced&) = delete;
		MaterialInstanced& operator=(const MaterialInstanced&) = delete;
		~MaterialInstanced() override = default;

		static void init();
	};

} // namespace rawrbox
