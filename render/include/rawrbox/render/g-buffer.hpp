#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	struct PosUVVertexData {
		float x = 0;
		float y = 0;
		float z = 0;
		float u = 0;
		float v = 0;

		PosUVVertexData() = default;
		PosUVVertexData(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& uv) : x(pos.x), y(pos.y), z(pos.z), u(uv.x), v(uv.y) {}
		PosUVVertexData(float _x, float _y, float _z, float _u, float _v, uint32_t _abgr) : x(_x), y(_y), z(_z), u(_u), v(_v) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout layout;
			layout
			    .begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .end();
			return layout;
		}
	};

	struct RenderTarget {
	public:
		bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
		bgfx::FrameBufferHandle buffer = BGFX_INVALID_HANDLE;

		RenderTarget(const RenderTarget&) = default;
		RenderTarget(RenderTarget&&) = default;
		RenderTarget& operator=(const RenderTarget&) = default;
		RenderTarget& operator=(RenderTarget&&) = delete;
		RenderTarget(uint32_t _width, uint32_t _height, bgfx::TextureFormat::Enum _format, uint64_t _flags) : texture(bgfx::createTexture2D(uint16_t(_width), uint16_t(_height), false, 1, _format, _flags)), buffer(bgfx::createFrameBuffer(1, &texture, true)) {}
		~RenderTarget() { RAWRBOX_DESTROY(buffer); }
	};

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
		static std::unique_ptr<rawrbox::RenderTarget> _linearDepth;
		static std::unique_ptr<rawrbox::RenderTarget> _shadows;
		static std::unique_ptr<rawrbox::GBufferUniforms> _uniforms;
		// -----

		static rawrbox::Matrix4x4 _proj;
		static rawrbox::Matrix4x4 _view;
		static float _projL[16];

		static void buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program);
		static void screenSpaceQuad(float _textureWidth, float _textureHeight, float _texelHalf, bool _originBottomLeft, float _width = 1.0F, float _height = 1.0F);

	public:
		static void init(const rawrbox::Vector2i& size);

		// UTILS ---
		static void setViewProjection(const rawrbox::Matrix4x4& view, const rawrbox::Matrix4x4& proj);

		static bgfx::FrameBufferHandle& getBuffer();
		static RenderTarget* getLinearDepth();
		static RenderTarget* getShadows();
		// -----

		static void render(const rawrbox::Vector2i& size);
		static void shutdown();
	};
} // namespace rawrbox
