
#include <rawrbox/render/g-buffer.hpp>

#include <bx/bx.h>
#include <bx/math.h>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader ldepth_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_sss_screenquad),
    BGFX_EMBEDDED_SHADER(fs_sss_linear_depth),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader shadows_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_sss_screenquad),
    BGFX_EMBEDDED_SHADER(fs_screen_space_shadows),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader combine_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_sss_screenquad),
    BGFX_EMBEDDED_SHADER(fs_sss_deferred_combine),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

#define GBUFFER_RT_COLOR       0
#define GBUFFER_RT_NORMAL      1
#define GBUFFER_RT_DEPTH       2
#define GBUFFER_RENDER_TARGETS 3

namespace rawrbox {
	// PRIVATE -----
	bgfx::FrameBufferHandle G_BUFFER::_gbuffer = BGFX_INVALID_HANDLE;
	bgfx::TextureHandle G_BUFFER::_gbufferTex[GBUFFER_RENDER_TARGETS] = {};

	bgfx::ProgramHandle G_BUFFER::_programLDepth = BGFX_INVALID_HANDLE;
	bgfx::ProgramHandle G_BUFFER::_programShadows = BGFX_INVALID_HANDLE;
	bgfx::ProgramHandle G_BUFFER::_programCombine = BGFX_INVALID_HANDLE;

	bgfx::UniformHandle G_BUFFER::_s_color = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle G_BUFFER::_s_normal = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle G_BUFFER::_s_depth = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle G_BUFFER::_s_shadows = BGFX_INVALID_HANDLE;

	// G-BUFFER ---
	std::unique_ptr<rawrbox::RenderTarget> G_BUFFER::_linearDepth = nullptr;
	std::unique_ptr<rawrbox::RenderTarget> G_BUFFER::_shadows = nullptr;

	std::unique_ptr<rawrbox::GBufferUniforms> G_BUFFER::_uniforms = nullptr;
	// -----

	rawrbox::Matrix4x4 G_BUFFER::_proj = {};
	rawrbox::Matrix4x4 G_BUFFER::_view = {};
	float G_BUFFER::_projL[16] = {};

	void G_BUFFER::screenSpaceQuad(float _textureWidth, float _textureHeight, float _texelHalf, bool _originBottomLeft, float _width, float _height) {
		auto layout = rawrbox::PosUVVertexData::vLayout();
		if (bgfx::getAvailTransientVertexBuffer(3, layout) != 3) return;

		bgfx::TransientVertexBuffer vb = {};
		bgfx::allocTransientVertexBuffer(&vb, 3, layout);
		auto vertex = std::bit_cast<rawrbox::PosUVVertexData*>(vb.data);

		const float minx = -_width;
		const float maxx = _width;
		const float miny = 0.F;
		const float maxy = _height * 2.F;

		const float texelHalfW = _texelHalf / _textureWidth;
		const float texelHalfH = _texelHalf / _textureHeight;
		const float minu = -1.F + texelHalfW;
		const float maxu = 1.F + texelHalfW;

		const float zz = 0.F;

		float minv = texelHalfH;
		float maxv = 2.F + texelHalfH;

		if (_originBottomLeft) {
			float temp = minv;
			minv = maxv;
			maxv = temp;

			minv -= 1.F;
			maxv -= 1.F;
		}

		vertex[0].x = minx;
		vertex[0].y = miny;
		vertex[0].z = zz;
		vertex[0].u = minu;
		vertex[0].v = minv;

		vertex[1].x = maxx;
		vertex[1].y = miny;
		vertex[1].z = zz;
		vertex[1].u = maxu;
		vertex[1].v = minv;

		vertex[2].x = maxx;
		vertex[2].y = maxy;
		vertex[2].z = zz;
		vertex[2].u = maxu;
		vertex[2].v = maxv;

		bgfx::setVertexBuffer(0, &vb);
	}

	// -----
	void G_BUFFER::buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, shaders[1].name);

		program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-GBUFFER] Failed to create shader");
	}

	void G_BUFFER::init(const rawrbox::Vector2i& size) {
		const uint64_t pointSampleFlags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT;

		auto w = static_cast<uint16_t>(size.x);
		auto h = static_cast<uint16_t>(size.y);

		_gbufferTex[GBUFFER_RT_COLOR] = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::BGRA8, pointSampleFlags);
		_gbufferTex[GBUFFER_RT_NORMAL] = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::BGRA8, pointSampleFlags);
		_gbufferTex[GBUFFER_RT_DEPTH] = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::D32F, pointSampleFlags);
		_gbuffer = bgfx::createFrameBuffer(BX_COUNTOF(_gbufferTex), _gbufferTex, true);

		// G-BUFFER ---
		_linearDepth = std::make_unique<rawrbox::RenderTarget>(w, h, bgfx::TextureFormat::R16F, pointSampleFlags);
		_shadows = std::make_unique<rawrbox::RenderTarget>(w, h, bgfx::TextureFormat::R16F, pointSampleFlags);

		_uniforms = std::make_unique<rawrbox::GBufferUniforms>();
		_uniforms->init();
		// -----

		_s_color = bgfx::createUniform("s_color", bgfx::UniformType::Sampler);   // Color gbuffer
		_s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler); // Normal gbuffer, Model's source normal
		_s_depth = bgfx::createUniform("s_depth", bgfx::UniformType::Sampler);   // Depth gbuffer
		_s_shadows = bgfx::createUniform("s_shadows", bgfx::UniformType::Sampler);

		bx::mtxProj(_projL, 60.F, static_cast<float>(size.x) / static_cast<float>(size.y), 0.01F, 100.0F, false);

		// SHADERS ---
		buildShader(ldepth_shaders, G_BUFFER::_programLDepth);
		buildShader(shadows_shaders, G_BUFFER::_programShadows);
		buildShader(combine_shaders, G_BUFFER::_programCombine);

		// -----
	}

	// UTILS ---
	void G_BUFFER::setViewProjection(const rawrbox::Matrix4x4& v, const rawrbox::Matrix4x4& p) {
		_view = v;
		_proj = p;
	}

	bgfx::FrameBufferHandle& G_BUFFER::getBuffer() {
		return _gbuffer;
	}

	rawrbox::RenderTarget* G_BUFFER::getLinearDepth() {
		return _linearDepth.get();
	}

	rawrbox::RenderTarget* G_BUFFER::getShadows() {
		return _shadows.get();
	}
	// -----

	void vec2Set(float* _v, float _x, float _y) {
		_v[0] = _x;
		_v[1] = _y;
	}

	void mat4Set(float* _m, const float* _src) {
		const uint32_t MAT4_FLOATS = 16;
		for (uint32_t ii = 0; ii < MAT4_FLOATS; ++ii) {
			_m[ii] = _src[ii];
		}
	}

	void G_BUFFER::render(const rawrbox::Vector2i& size) {
		if (_uniforms == nullptr) return;

		const bgfx::Caps* caps = bgfx::getCaps();
		const bgfx::RendererType::Enum renderer = bgfx::getRendererType();

		// Update uniforms --
		_uniforms->m_displayShadows = 1.F;
		_uniforms->m_frameIdx = 0.F;
		_uniforms->m_shadowRadius = 25.0F;
		_uniforms->m_shadowSteps = 8;
		_uniforms->m_useNoiseOffset = 1.0F;
		_uniforms->m_contactShadowsMode = 0;
		_uniforms->m_useScreenSpaceRadius = 0.0F;

		mat4Set(_uniforms->m_worldToView, _view.data());
		mat4Set(_uniforms->m_viewToProj, _proj.data());

		// from assao sample, cs_assao_prepare_depths.sc
		{
			// float depthLinearizeMul = ( clipFar * clipNear ) / ( clipFar - clipNear );
			// float depthLinearizeAdd = clipFar / ( clipFar - clipNear );
			// correct the handedness issue. need to make sure this below is correct, but I think it is.

			float depthLinearizeMul = -_projL[3 * 4 + 2];
			float depthLinearizeAdd = _projL[2 * 4 + 2];

			if (depthLinearizeMul * depthLinearizeAdd < 0) {
				depthLinearizeAdd = -depthLinearizeAdd;
			}

			vec2Set(_uniforms->m_depthUnpackConsts, depthLinearizeMul, depthLinearizeAdd);

			float tanHalfFOVY = 1.0F / _projL[1 * 4 + 1]; // = tanf( drawContext.Camera.GetYFOV( ) * 0.5f );
			float tanHalfFOVX = 1.0F / _projL[0];         // = tanHalfFOVY * drawContext.Camera.GetAspect( );

			if (bgfx::getRendererType() == bgfx::RendererType::OpenGL) {
				vec2Set(_uniforms->m_ndcToViewMul, tanHalfFOVX * 2.0f, tanHalfFOVY * 2.0f);
				vec2Set(_uniforms->m_ndcToViewAdd, tanHalfFOVX * -1.0f, tanHalfFOVY * -1.0f);
			} else {
				vec2Set(_uniforms->m_ndcToViewMul, tanHalfFOVX * 2.0f, tanHalfFOVY * -2.0f);
				vec2Set(_uniforms->m_ndcToViewAdd, tanHalfFOVX * -1.0f, tanHalfFOVY * 1.0f);
			}
		}

		/*{
			float lightPosition[4];
			bx::memCopy(lightPosition, m_lightModel.position, 3 * sizeof(float));
			lightPosition[3] = 1.0f;
			float viewSpaceLightPosition[4];
			bx::vec4MulMtx(viewSpaceLightPosition, lightPosition, m_view);
			bx::memCopy(m_uniforms.m_lightPosition, viewSpaceLightPosition, 3 * sizeof(float));
		}*/
		// ----

		bgfx::ViewId oldview = rawrbox::CURRENT_VIEW_ID;

		std::array<float, 16> orthoProj = {};
		bx::mtxOrtho(orthoProj.data(), 0.0F, 1.0F, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F, caps->homogeneousDepth);

		auto lBuffer = rawrbox::G_BUFFER::getLinearDepth();
		if (lBuffer != nullptr) {
			rawrbox::CURRENT_VIEW_ID = rawrbox::GBUFFER_COLOR_VIEW_ID;

			bgfx::setViewRect(rawrbox::CURRENT_VIEW_ID, 0, 0, static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y));
			bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, orthoProj.data());
			bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, rawrbox::G_BUFFER::getLinearDepth()->buffer);

			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);
			bgfx::setTexture(0, _s_depth, _gbufferTex[GBUFFER_RT_DEPTH]);
			_uniforms->submit();

			screenSpaceQuad(size.x, size.y, bgfx::RendererType::Direct3D9 == renderer ? 0.5F : 0.0F, caps->originBottomLeft);
			bgfx::submit(rawrbox::CURRENT_VIEW_ID, _programLDepth);
		}

		auto shadowBuffer = rawrbox::G_BUFFER::getShadows();
		if (shadowBuffer != nullptr) {
			rawrbox::CURRENT_VIEW_ID = rawrbox::GBUFFER_COLOR_VIEW_ID + 1;

			bgfx::setViewRect(rawrbox::CURRENT_VIEW_ID, 0, 0, static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y));
			bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, orthoProj.data());
			bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, rawrbox::G_BUFFER::getShadows()->buffer);
			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);

			bgfx::setTexture(0, _s_depth, rawrbox::G_BUFFER::getLinearDepth()->texture);
			_uniforms->submit();

			screenSpaceQuad(size.x, size.y, bgfx::RendererType::Direct3D9 == renderer ? 0.5F : 0.0F, caps->originBottomLeft);
			bgfx::submit(rawrbox::CURRENT_VIEW_ID, _programShadows);

			// Done
			rawrbox::CURRENT_VIEW_ID = oldview;
		}

		// Shade gbuffer
		rawrbox::CURRENT_VIEW_ID = rawrbox::GBUFFER_COLOR_VIEW_ID + 2;
		bgfx::setViewRect(rawrbox::CURRENT_VIEW_ID, 0, 0, static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y));
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, orthoProj.data());
		bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, BGFX_INVALID_HANDLE);
		bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);

		bgfx::setTexture(0, _s_color, _gbufferTex[GBUFFER_RT_COLOR]);
		bgfx::setTexture(1, _s_normal, _gbufferTex[GBUFFER_RT_NORMAL]);
		bgfx::setTexture(2, _s_depth, _linearDepth->texture);
		bgfx::setTexture(3, _s_shadows, _shadows->texture);

		_uniforms->submit();
		screenSpaceQuad(size.x, size.y, bgfx::RendererType::Direct3D9 == renderer ? 0.5F : 0.0F, caps->originBottomLeft);
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, _programCombine);
		// ---

		// Done
		rawrbox::CURRENT_VIEW_ID = oldview;
	}

	void G_BUFFER::shutdown() {
		RAWRBOX_DESTROY(_gbuffer);

		RAWRBOX_DESTROY(_programCombine);
		RAWRBOX_DESTROY(_programLDepth);
		RAWRBOX_DESTROY(_programShadows);

		RAWRBOX_DESTROY(_s_color);
		RAWRBOX_DESTROY(_s_normal);
		RAWRBOX_DESTROY(_s_depth);
		RAWRBOX_DESTROY(_s_shadows);

		for (auto i : _gbufferTex)
			RAWRBOX_DESTROY(i);

		_linearDepth.reset();
		_shadows.reset();
		_uniforms.reset();
	}

} // namespace rawrbox
