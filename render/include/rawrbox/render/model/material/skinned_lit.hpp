#pragma once

#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/material/lit.hpp>
#include <rawrbox/render/shader_defines.h>

#include <bgfx/bgfx.h>
#include <fmt/format.h>
#include <generated/shaders/render/all.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_skinned_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_skinned_lit),
    BGFX_EMBEDDED_SHADER(fs_model_skinned_lit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrBox {

	class MaterialSkinnedLit : public rawrBox::MaterialLit {

	public:
		using MaterialLit::MaterialLit;

		void process(std::shared_ptr<rawrBox::Mesh> mesh) override {
			rawrBox::MaterialLit::process(mesh);
		}

		void upload() override {
			this->buildShader(model_skinned_lit_shaders, "model_skinned_lit");
		}
	};
} // namespace rawrBox
