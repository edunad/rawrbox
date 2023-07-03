#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/shader_defines.hpp>

#include <bgfx/bgfx.h>

#include <array>

#define FORWARD_RT_DEPTH 0
#define FORWARD_RT_COUNT 1

// https://github.com/bcrusco/Forward-Plus-Renderer/blob/master/Forward-Plus/Forward-Plus/source/main.cpp
namespace rawrbox {

	struct Light {
		rawrbox::Vector4f position = {};
		rawrbox::Colorf color = rawrbox::Colors::White;
		rawrbox::Vector4f paddingAndRadius = {};

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float) // Position
			    .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float) // Color
			    .add(bgfx::Attrib::TexCoord2, 4, bgfx::AttribType::Float) // Padding
			    .end();
			return l;
		};
	};

	class FORWARD_PLUS {
	protected:
		static float _workGroupsX;
		static float _workGroupsY;

		static bgfx::FrameBufferHandle _gbuffer;
		static std::array<bgfx::TextureHandle, FORWARD_RT_COUNT> _gbufferTex;

		static bgfx::DynamicVertexBufferHandle _lightBuffer;
		static bgfx::IndexBufferHandle _visibleLightBuffer;

		static bgfx::ProgramHandle _programLightCull;

		static bgfx::UniformHandle _s_depth;

		static bgfx::UniformHandle _u_lightSettings;

		// NOLINTBEGIN(*)
		static void buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program);
		static void buildComputeShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program);
		// NOLINTEND(*)

		/*static rawrbox::Matrix4x4 _orthoProj;

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


		static bx::Aabb calculateAABB(const rawrbox::LightBase& light);

		static void lightPass(const rawrbox::Vector2i& size);
		static void combine(const rawrbox::Vector2i& size);*/

	public:
		static void init(const rawrbox::Vector2i& size);
		static void render(const rawrbox::Vector2i& size);
		static void update();

		static void shutdown();
	};
} // namespace rawrbox
