#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	struct GBufferUniforms {
	private:
		bgfx::UniformHandle _params;

	public:
		enum { NumVec4 = 12 };
		void init() {
			this->_params = bgfx::createUniform("u_params", bgfx::UniformType::Vec4, NumVec4);
		};

		void submit() const {
			bgfx::setUniform(_params, m_params, NumVec4);
		}

		~GBufferUniforms() {
			RAWRBOX_DESTROY(_params);
		}

		union {
			struct
			{
				/* 0    */ struct {
					float m_frameIdx;
					float m_shadowRadius;
					float m_shadowSteps;
					float m_useNoiseOffset;
				};
				/* 1    */ struct {
					float m_depthUnpackConsts[2];
					float m_contactShadowsMode;
					float m_useScreenSpaceRadius;
				};
				/* 2    */ struct {
					float m_ndcToViewMul[2];
					float m_ndcToViewAdd[2];
				};
				/* 3    */ struct {
					float m_lightPosition[3];
					float m_displayShadows;
				};
				/* 4-7  */ struct {
					float m_worldToView[16];
				}; // built-in u_view will be transform for quad during screen passes
				/* 8-11 */ struct {
					float m_viewToProj[16];
				}; // built-in u_proj will be transform for quad during screen passes
			};

			float m_params[NumVec4 * 4];
		};
	};

	class G_BUFFER {
	protected:
		static bgfx::FrameBufferHandle _gbuffer;
		static bgfx::TextureHandle _gbufferTex[3];

		static bgfx::ProgramHandle _programLDepth;
		static bgfx::ProgramHandle _programShadows;
		static bgfx::ProgramHandle _programCombine;

		static bgfx::UniformHandle _s_color;
		static bgfx::UniformHandle _s_normal;
		static bgfx::UniformHandle _s_depth;
		static bgfx::UniformHandle _s_shadows;

		// G-BUFFER ---
		static std::unique_ptr<rawrbox::TextureRender> _linearDepth;
		static std::unique_ptr<rawrbox::TextureRender> _shadows;
		static std::unique_ptr<rawrbox::GBufferUniforms> _uniforms;
		// -----

		static void updateUniforms();
		static void buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program);

	public:
		static void init(const rawrbox::Vector2i& size);

		// UTILS ---
		static bgfx::FrameBufferHandle& getBuffer();
		static rawrbox::TextureRender* getLinearDepth();
		static rawrbox::TextureRender* getShadows();
		// -----

		static void render(const rawrbox::Vector2i& size);
		static void shutdown();
	};
} // namespace rawrbox
