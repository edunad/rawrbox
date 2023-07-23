#pragma once
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>

#include <stdexcept>

namespace rawrbox {

	enum G_BUFFER : size_t {
		DIFFUSE,
		NORMAL,
		EMISSION_SPECULAR,

		BITMASK, // For decals / shadows

		DEPTH,

		COUNT
	};

	class RendererDeferred : public rawrbox::RendererBase {
	protected:
		static constexpr uint64_t GBUFFER_FLAGS = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
							  BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP |
							  BGFX_SAMPLER_V_CLAMP;

		std::array<bgfx::UniformHandle, G_BUFFER::COUNT> _gBufferSamplers = {
		    bgfx::kInvalidHandle,
		    bgfx::kInvalidHandle,
		    bgfx::kInvalidHandle,
		    bgfx::kInvalidHandle,
		    bgfx::kInvalidHandle,
		};

		bgfx::FrameBufferHandle _gbuffer = BGFX_INVALID_HANDLE;
		bgfx::FrameBufferHandle _gbuffer_acc = BGFX_INVALID_HANDLE; // Accumulator

		bgfx::TextureHandle _depthCopy = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle _accTexture = BGFX_INVALID_HANDLE;

		bgfx::ProgramHandle _worldLightProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _lightProgram = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _light_index = BGFX_INVALID_HANDLE;

		bgfx::VertexBufferHandle _bbox_vb = BGFX_INVALID_HANDLE;
		bgfx::IndexBufferHandle _bbox_ib = BGFX_INVALID_HANDLE;

	public:
		RendererDeferred() = default;
		RendererDeferred(const RendererDeferred &) = delete;
		RendererDeferred(RendererDeferred &&) = delete;
		RendererDeferred &operator=(const RendererDeferred &) = delete;
		RendererDeferred &operator=(RendererDeferred &&) = delete;
		~RendererDeferred() override;

		static bool supported();

		void init(const rawrbox::Vector2i &size) override;
		void resize(const rawrbox::Vector2i &size) override;

		void render() override;
		void bindRenderUniforms() override;

		void applyLight(size_t indx);
		void finalRender() override;
	};
} // namespace rawrbox
