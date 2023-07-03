#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/base.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <bgfx/bgfx.h>
#include <bx/bounds.h>

#define GBUFFER_RT_ALBEDO      0
#define GBUFFER_RT_NORMAL      1
#define GBUFFER_RT_DEPTH       2
#define GBUFFER_RENDER_TARGETS 3

namespace rawrbox {
	class G_BUFFER {
	protected:
		static bgfx::FrameBufferHandle _gbuffer;
		static std::array<bgfx::TextureHandle, GBUFFER_RENDER_TARGETS> _gbufferTex;
		static rawrbox::Matrix4x4 _orthoProj;

		static bgfx::ProgramHandle _programCombine;
		static bgfx::ProgramHandle _programLight;

		static bgfx::UniformHandle _s_albedo;
		static bgfx::UniformHandle _s_normal;
		static bgfx::UniformHandle _s_depth;

		static bgfx::UniformHandle _s_light;

		// Light ----
		static std::unique_ptr<rawrbox::TextureRender> _lightBuffer;

		static bgfx::UniformHandle u_mtx;
		static bgfx::UniformHandle u_lightPosition;
		static bgfx::UniformHandle u_lightData;

		// ------

		// NOLINTBEGIN(*)
		static void buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program);
		// NOLINTEND(*)

		static bx::Aabb calculateAABB(const rawrbox::LightBase& light);

		static void lightPass(const rawrbox::Vector2i& size);
		static void combine(const rawrbox::Vector2i& size);

	public:
		static void init(const rawrbox::Vector2i& size);

		// UTILS ---
		static bgfx::FrameBufferHandle& getBuffer();
		// -----

		static void render(const rawrbox::Vector2i& size);
		static void shutdown();
	};
} // namespace rawrbox
