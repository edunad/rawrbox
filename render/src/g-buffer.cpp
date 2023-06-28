
#include <rawrbox/render/g-buffer.hpp>
#include <rawrbox/render/utils/render.hpp>

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
	bgfx::TextureHandle G_BUFFER::_gbufferTex[GBUFFER_RENDER_TARGETS];

	bgfx::ProgramHandle G_BUFFER::_programLDepth = BGFX_INVALID_HANDLE;
	bgfx::ProgramHandle G_BUFFER::_programShadows = BGFX_INVALID_HANDLE;
	bgfx::ProgramHandle G_BUFFER::_programCombine = BGFX_INVALID_HANDLE;

	bgfx::UniformHandle G_BUFFER::_s_color = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle G_BUFFER::_s_normal = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle G_BUFFER::_s_depth = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle G_BUFFER::_s_shadows = BGFX_INVALID_HANDLE;

	// G-BUFFER ---
	std::unique_ptr<rawrbox::TextureRender> G_BUFFER::_linearDepth = nullptr;
	std::unique_ptr<rawrbox::TextureRender> G_BUFFER::_shadows = nullptr;

	std::unique_ptr<rawrbox::GBufferUniforms> G_BUFFER::_uniforms = nullptr;
	// -----

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

		// Samples ---
		_s_color = bgfx::createUniform("s_color", bgfx::UniformType::Sampler);   // Color gbuffer
		_s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler); // Normal gbuffer, Model's source normal
		_s_depth = bgfx::createUniform("s_depth", bgfx::UniformType::Sampler);   // Depth gbuffer
		_s_shadows = bgfx::createUniform("s_shadows", bgfx::UniformType::Sampler);
		// -----

		// Buffers ----
		_gbufferTex[GBUFFER_RT_COLOR] = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::RGBA8, pointSampleFlags);
		_gbufferTex[GBUFFER_RT_NORMAL] = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::RGBA8, pointSampleFlags);
		_gbufferTex[GBUFFER_RT_DEPTH] = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::D32F, pointSampleFlags);
		_gbuffer = bgfx::createFrameBuffer(BX_COUNTOF(_gbufferTex), _gbufferTex, true);

		// G-BUFFER ---
		_linearDepth = std::make_unique<rawrbox::TextureRender>(size, rawrbox::GBUFFER_L_DEPTH_VIEW_ID);
		_linearDepth->setFlags(pointSampleFlags);
		_linearDepth->upload(bgfx::TextureFormat::R16F);

		_shadows = std::make_unique<rawrbox::TextureRender>(size, rawrbox::GBUFFER_SHADOW_VIEW_ID);
		_shadows->setFlags(pointSampleFlags);
		_shadows->upload(bgfx::TextureFormat::R16F);

		_uniforms = std::make_unique<rawrbox::GBufferUniforms>();
		_uniforms->init();
		// -----

		// SHADERS ---
		buildShader(ldepth_shaders, G_BUFFER::_programLDepth);
		buildShader(shadows_shaders, G_BUFFER::_programShadows);
		buildShader(combine_shaders, G_BUFFER::_programCombine);
		// -----
	}

	// UTILS ---
	bgfx::FrameBufferHandle& G_BUFFER::getBuffer() {
		return _gbuffer;
	}

	rawrbox::TextureRender* G_BUFFER::getLinearDepth() {
		return _linearDepth.get();
	}

	rawrbox::TextureRender* G_BUFFER::getShadows() {
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

	void G_BUFFER::updateUniforms() {
		if (_uniforms == nullptr) return;

		// Update uniforms --
		_uniforms->m_displayShadows = 0.F;
		_uniforms->m_frameIdx = float(rawrbox::BGFX_FRAME % 8);
		_uniforms->m_shadowRadius = 0.65F;
		_uniforms->m_shadowSteps = 8;
		_uniforms->m_useNoiseOffset = 1.0F;
		_uniforms->m_contactShadowsMode = 1.5F;
		_uniforms->m_useScreenSpaceRadius = 0.0F;

		auto& view = rawrbox::MAIN_CAMERA->getViewMtx();
		auto& proj = rawrbox::MAIN_CAMERA->getProjMtx();

		mat4Set(_uniforms->m_worldToView, view.data());
		mat4Set(_uniforms->m_viewToProj, proj.data());

		// from assao sample, cs_assao_prepare_depths.sc
		{
			// float depthLinearizeMul = ( clipFar * clipNear ) / ( clipFar - clipNear );
			// float depthLinearizeAdd = clipFar / ( clipFar - clipNear );
			// correct the handedness issue. need to make sure this below is correct, but I think it is.

			float depthLinearizeMul = -proj[3 * 4 + 2];
			float depthLinearizeAdd = proj[2 * 4 + 2];

			if (depthLinearizeMul * depthLinearizeAdd < 0) {
				depthLinearizeAdd = -depthLinearizeAdd;
			}

			vec2Set(_uniforms->m_depthUnpackConsts, depthLinearizeMul, depthLinearizeAdd);

			float tanHalfFOVY = 1.0F / proj[1 * 4 + 1]; // = tanf( drawContext.Camera.GetYFOV( ) * 0.5f );
			float tanHalfFOVX = 1.0F / proj[0];         // = tanHalfFOVY * drawContext.Camera.GetAspect( );

			if (bgfx::getRendererType() == bgfx::RendererType::OpenGL) {
				vec2Set(_uniforms->m_ndcToViewMul, tanHalfFOVX * 2.0f, tanHalfFOVY * 2.0f);
				vec2Set(_uniforms->m_ndcToViewAdd, tanHalfFOVX * -1.0f, tanHalfFOVY * -1.0f);
			} else {
				vec2Set(_uniforms->m_ndcToViewMul, tanHalfFOVX * 2.0f, tanHalfFOVY * -2.0f);
				vec2Set(_uniforms->m_ndcToViewAdd, tanHalfFOVX * -1.0f, tanHalfFOVY * 1.0f);
			}
		}

		{
			auto frame = static_cast<float>(rawrbox::BGFX_FRAME) * 0.01F;

			float lightPosition[4] = {std::cos(frame) * 2.F, 0.5F, std::sin(frame) * 2.F, 1.F};
			float viewSpaceLightPosition[4];
			bx::vec4MulMtx(viewSpaceLightPosition, lightPosition, view.data());
			bx::memCopy(_uniforms->m_lightPosition, viewSpaceLightPosition, 3 * sizeof(float));
		}

		// ----
	}

	void G_BUFFER::render(const rawrbox::Vector2i& size) {
		if (_linearDepth == nullptr || _shadows == nullptr || _uniforms == nullptr) return;

		const bgfx::Caps* caps = bgfx::getCaps();
		const bgfx::RendererType::Enum renderer = bgfx::getRendererType();

		bgfx::ViewId oldview = rawrbox::CURRENT_VIEW_ID;

		std::array<float, 16> orthoProj = {};
		bx::mtxOrtho(orthoProj.data(), 0.0F, 1.0F, 1.0F, 0.0F, 0.0F, 1.0F, 0.0F, caps->homogeneousDepth);

		updateUniforms();
		bgfx::discard();

		// Convert depth to linear depth for shadow depth compare
		{
			rawrbox::CURRENT_VIEW_ID = rawrbox::GBUFFER_L_DEPTH_VIEW_ID;

			bgfx::setViewName(rawrbox::CURRENT_VIEW_ID, "linear depth");
			bgfx::setViewRect(rawrbox::CURRENT_VIEW_ID, 0, 0, size.x, size.y);
			bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, orthoProj.data());
			bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, _linearDepth->getBuffer());

			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);
			bgfx::setTexture(0, _s_depth, _gbufferTex[GBUFFER_RT_DEPTH]);

			_uniforms->submit();
			rawrbox::RenderUtils::renderScreenQuad(size);

			bgfx::submit(rawrbox::CURRENT_VIEW_ID, _programLDepth);
		}

		// Do screen space shadows
		{
			rawrbox::CURRENT_VIEW_ID = rawrbox::GBUFFER_SHADOW_VIEW_ID;

			bgfx::setViewName(rawrbox::CURRENT_VIEW_ID, "screen space shadows");
			bgfx::setViewRect(rawrbox::CURRENT_VIEW_ID, 0, 0, size.x, size.y);
			bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, orthoProj.data());
			bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, _shadows->getBuffer());
			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);

			bgfx::setTexture(0, _s_depth, _linearDepth->getHandle());

			_uniforms->submit();
			rawrbox::RenderUtils::renderScreenQuad(size);

			bgfx::submit(rawrbox::CURRENT_VIEW_ID, _programShadows);

			// Done
			rawrbox::CURRENT_VIEW_ID = oldview;
		}

		// Shade gbuffer
		{
			rawrbox::CURRENT_VIEW_ID = rawrbox::GBUFFER_COMBINE_VIEW_ID;
			bgfx::setViewName(rawrbox::CURRENT_VIEW_ID, "combine");
			bgfx::setViewRect(rawrbox::CURRENT_VIEW_ID, 0, 0, size.x, size.y);
			bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, orthoProj.data());
			bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, BGFX_INVALID_HANDLE);
			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS);

			bgfx::setTexture(0, _s_color, _gbufferTex[GBUFFER_RT_COLOR]);
			bgfx::setTexture(1, _s_normal, _gbufferTex[GBUFFER_RT_NORMAL]);
			bgfx::setTexture(2, _s_depth, _linearDepth->getHandle());
			bgfx::setTexture(3, _s_shadows, _shadows->getHandle());

			_uniforms->submit();
			rawrbox::RenderUtils::renderScreenQuad(size);

			bgfx::submit(rawrbox::CURRENT_VIEW_ID, _programCombine);
		}
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
