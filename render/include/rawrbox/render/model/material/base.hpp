#pragma once

#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>
#include <rawrbox/render/util/uniforms.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

#include <concepts>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace rawrBox {
	class MaterialBase {

	protected:
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		std::unordered_map<std::string, bgfx::UniformHandle> _uniforms = {};

	public:
		bgfx::VertexLayout vLayout;

		MaterialBase() {
			this->vLayout.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    // -------------
			    .add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Uint8, true, true)
			    // -------------
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)

			    // Bones
			    .add(bgfx::Attrib::Indices, rawrBox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Uint8, false, true)
			    .add(bgfx::Attrib::Weight, rawrBox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Float)
			    // -----

			    .skip(sizeof(int)) // Skip index
			    // -----
			    .end();
		};

		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;

		virtual ~MaterialBase() {
			RAWRBOX_DESTROY(this->_program);

			for (auto uni : this->_uniforms)
				RAWRBOX_DESTROY(uni.second);

			this->_uniforms.clear();
		}

		virtual bool hasUniform(const std::string id) {
			return this->_uniforms.find(id) != this->_uniforms.end();
		}

		virtual bgfx::UniformHandle getUniform(const std::string id) {
			auto fnd = this->_uniforms.find(id);
			if (fnd == this->_uniforms.end()) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Uniform '{}' not found!", id));

			return fnd->second;
		}

		virtual void buildShader(const bgfx::EmbeddedShader shaders[], const std::string& name) {
			bgfx::RendererType::Enum type = bgfx::getRendererType();
			bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, fmt::format("vs_{}", name).c_str());
			bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, fmt::format("fs_{}", name).c_str());

			this->_program = bgfx::createProgram(vsh, fsh, true);
			if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-MaterialBase] Failed to create shader");

			this->registerUniforms();
		}

		virtual void registerUniforms() {
			this->_uniforms = {
			    {"u_viewPos", bgfx::createUniform("u_viewPos", bgfx::UniformType::Vec4, 3)},

			    // LIT / UNLIT
			    {"s_texColor", bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler)},
			    {"s_texSpecularColor", bgfx::createUniform("s_texSpecularColor", bgfx::UniformType::Sampler)},

			    {"u_texSpecularShininess", bgfx::createUniform("u_texSpecularShininess", bgfx::UniformType::Vec4, 1)},
			    {"u_colorOffset", bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4)},

			    {"u_lightsSetting", bgfx::createUniform("u_lightsSetting", bgfx::UniformType::Vec4, 2)},
			    {"u_lightsPosition", bgfx::createUniform("u_lightsPosition", bgfx::UniformType::Vec4, rawrBox::MAX_LIGHTS)},
			    {"u_lightsData", bgfx::createUniform("u_lightsData", bgfx::UniformType::Mat4, rawrBox::MAX_LIGHTS)},

			    // SKINNED
			    {"u_bones", bgfx::createUniform("u_bones", bgfx::UniformType::Mat4, rawrBox::MAX_BONES_PER_MODEL)},

			    // SPRITE
			    {"u_colorOffset", bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4)},
			    {"u_sprite_pos", bgfx::createUniform("u_sprite_pos", bgfx::UniformType::Vec4, 3)}, // ¯\_(ツ)_/¯ hate it
			};
		};

		virtual void preProcess(const rawrBox::Vector3f& camPos) {
			std::array pos = {camPos.x, camPos.y, camPos.z};
			bgfx::setUniform(this->getUniform("u_viewPos"), pos.data());
		};

		virtual void process(std::shared_ptr<rawrBox::Mesh> mesh){};
		virtual void postProcess() {
			bgfx::submit(rawrBox::CURRENT_VIEW_ID, this->_program);
		}

		virtual void upload() = 0;
	};
} // namespace rawrBox
