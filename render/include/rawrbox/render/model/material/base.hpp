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

		virtual void registerUniform(const std::string id, bgfx::UniformHandle handle) {
			auto fnd = this->_uniforms.find(id);
			if (fnd != this->_uniforms.end()) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Failed to register uniform '{}', already created!", id));
			this->_uniforms[id] = handle;
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
			// Default uniforms
			this->registerUniform("u_viewPos", bgfx::createUniform("u_viewPos", bgfx::UniformType::Vec4, 3));
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
