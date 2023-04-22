
#include <rawrbox/math/quaternion.hpp>
#include <rawrbox/render/model/sprite.h>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>
#include <rawrbox/render/util/uniforms.hpp>

#include <bx/math.h>
#include <generated/shaders/render/all.h>

#define BGFX_STATE_DEFAULT_SPRITE (0 | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)

static const bgfx::EmbeddedShader sprite_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_sprite),
    BGFX_EMBEDDED_SHADER(fs_sprite),
    BGFX_EMBEDDED_SHADER_END()};

namespace rawrBox {
	Sprite::Sprite() {
		this->_vLayout.begin()
		    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		    .end();
	}

	Sprite::~Sprite() {
		ModelBase::~ModelBase();

		RAWRBOX_DESTROY(this->_spritePos);
		RAWRBOX_DESTROY(this->_texColor);
		RAWRBOX_DESTROY(this->_offsetColor);
	}

	void Sprite::upload() {
		ModelBase::upload();

		// Setup shader -----
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(sprite_shaders, type, "vs_sprite");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(sprite_shaders, type, "fs_sprite");

		this->_program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-Model] Failed to bind shader");
		// -----------------

		this->_spritePos = bgfx::createUniform("u_sprite_pos", bgfx::UniformType::Vec4); // ¯\_(ツ)_/¯ hate it

		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		this->_offsetColor = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);
	}

	void Sprite::draw(const rawrBox::Vector3f& camPos) {
		ModelBase::draw(camPos);

		for (auto& mesh : this->_meshes) {
			auto& data = mesh->getData();

			if (data->texture != nullptr && data->texture->valid() && !data->wireframe) {
				bgfx::setTexture(0, this->_texColor, data->texture->getHandle());
			} else {
				bgfx::setTexture(0, this->_texColor, rawrBox::MISSING_TEXTURE->getHandle());
			}

			UniformUtils::setUniform(this->_offsetColor, data->color);
			UniformUtils::setUniform(this->_spritePos, rawrBox::Quaternion(data->offsetMatrix[12], data->offsetMatrix[13], data->offsetMatrix[14], data->offsetMatrix[15]));

			bgfx::setTransform(this->_matrix.data());

			bgfx::setVertexBuffer(0, this->_vbh, data->baseVertex, static_cast<uint32_t>(data->vertices.size()));
			bgfx::setIndexBuffer(this->_ibh, data->baseIndex, static_cast<uint32_t>(data->indices.size()));

			uint64_t flags = BGFX_STATE_DEFAULT_SPRITE;
			if (data->wireframe) flags |= BGFX_STATE_PT_LINES;

			bgfx::setState(flags, 0);
			bgfx::submit(rawrBox::CURRENT_VIEW_ID, this->_program);
		}
	}
} // namespace rawrBox
