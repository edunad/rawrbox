#pragma once

#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/material/unlit.hpp>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>

#include <bgfx/bgfx.h>
#include <fmt/format.h>
#include <generated/shaders/render/all.h>

#include <vector>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_skinned_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_skinned_unlit),
    BGFX_EMBEDDED_SHADER(fs_model_skinned_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrBox {

	class MaterialSkinnedUnlit : public rawrBox::MaterialUnlit {

	public:
		using MaterialUnlit::MaterialUnlit;

		void registerUniforms() override {
			rawrBox::MaterialUnlit::registerUniforms();

			this->registerUniform("u_displayBone", bgfx::createUniform("u_displayBone", bgfx::UniformType::Vec4, 1));
			this->registerUniform("u_bones", bgfx::createUniform("u_bones", bgfx::UniformType::Mat4, 200));
		}

		void process(std::shared_ptr<rawrBox::Mesh> mesh) override {
			rawrBox::MaterialUnlit::process(mesh);

			/*auto b = mesh->parent["Head"];
			float a[1] = {static_cast<float>(b.boneId)};

			bgfx::setUniform(this->getUniform("u_displayBone"), a);
			bgfx::dbgTextPrintf(1, 2, 0x0f, fmt::format("Current Bone: {}", b.name).c_str());*/
		}

		void upload() override {
			this->buildShader(model_skinned_unlit_shaders, "model_skinned_unlit");
		}
	};
} // namespace rawrBox
