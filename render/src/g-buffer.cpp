
#include <rawrbox/render/g-buffer.hpp>
#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <bx/bx.h>
#include <bx/debug.h>
#include <bx/math.h>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader light_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_screenquad),
    BGFX_EMBEDDED_SHADER(fs_light_pass),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader combine_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_screenquad),
    BGFX_EMBEDDED_SHADER(fs_combine),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	// PRIVATE -----
	bgfx::FrameBufferHandle G_BUFFER::_gbuffer = BGFX_INVALID_HANDLE;
	std::array<bgfx::TextureHandle, GBUFFER_RENDER_TARGETS> G_BUFFER::_gbufferTex;

	rawrbox::Matrix4x4 G_BUFFER::_orthoProj = {};

	bgfx::ProgramHandle G_BUFFER::_programCombine = BGFX_INVALID_HANDLE;
	bgfx::ProgramHandle G_BUFFER::_programLight = BGFX_INVALID_HANDLE;

	bgfx::UniformHandle G_BUFFER::_s_albedo = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle G_BUFFER::_s_normal = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle G_BUFFER::_s_depth = BGFX_INVALID_HANDLE;

	bgfx::UniformHandle G_BUFFER::_s_light = BGFX_INVALID_HANDLE;

	// Light ----
	std::unique_ptr<rawrbox::TextureRender> G_BUFFER::_lightBuffer = nullptr;

	bgfx::UniformHandle G_BUFFER::u_mtx = BGFX_INVALID_HANDLE;

	bgfx::UniformHandle G_BUFFER::u_lightPosition = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle G_BUFFER::u_lightData = BGFX_INVALID_HANDLE;
	// -----

	// NOLINTBEGIN(*)
	void G_BUFFER::buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, shaders[1].name);

		program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-GBUFFER] Failed to create shader");
	}
	// NOLINTEND(*)

	void G_BUFFER::init(const rawrbox::Vector2i& size) {
		const uint64_t pointSampleFlags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

		auto w = static_cast<uint16_t>(size.x);
		auto h = static_cast<uint16_t>(size.y);

		// Samples ---
		_s_albedo = bgfx::createUniform("s_albedo", bgfx::UniformType::Sampler); // Color gbuffer
		_s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler); // Normal gbuffer
		_s_depth = bgfx::createUniform("s_depth", bgfx::UniformType::Sampler);   // Depth gbuffer

		_s_light = bgfx::createUniform("s_light", bgfx::UniformType::Sampler); // Light
		// -----

		// Uniforms --
		// LIGHTS
		_lightBuffer = std::make_unique<rawrbox::TextureRender>(size, rawrbox::GBUFFER_LIGHT_VIEW_ID, false);
		_lightBuffer->setFlags(pointSampleFlags);
		_lightBuffer->upload(bgfx::TextureFormat::RGBA8);

		u_mtx = bgfx::createUniform("u_mtx", bgfx::UniformType::Mat4);
		u_lightPosition = bgfx::createUniform("u_lightPosition", bgfx::UniformType::Vec4);
		u_lightData = bgfx::createUniform("u_lightData", bgfx::UniformType::Mat4);
		// -----

		// Buffers ----
		_gbufferTex[GBUFFER_RT_ALBEDO] = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::RGBA8, pointSampleFlags);
		_gbufferTex[GBUFFER_RT_NORMAL] = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::RG16, pointSampleFlags);
		_gbufferTex[GBUFFER_RT_DEPTH] = bgfx::createTexture2D(w, h, false, 1, bgfx::TextureFormat::D24, pointSampleFlags);

		_gbuffer = bgfx::createFrameBuffer(GBUFFER_RENDER_TARGETS, _gbufferTex.data(), true);
		// -----

		// Camera --
		bx::mtxOrtho(_orthoProj.data(), 0.0F, 1.0F, 1.0F, 0.0F, 0.0F, 100.0F, 0.0F, bgfx::getCaps()->homogeneousDepth);
		// -----

		// SHADERS ---
		buildShader(combine_shaders, _programCombine);
		buildShader(light_shaders, _programLight);
		// -----
	}

	// UTILS ---
	bgfx::FrameBufferHandle& G_BUFFER::getBuffer() {
		return _gbuffer;
	}
	// -----

	bx::Aabb G_BUFFER::calculateAABB(const rawrbox::LightBase& light) {
		bx::Aabb aabb;

		auto& color = light.getDiffuseColor();
		auto& pos = light.getPosMatrix();

		float lightMax = std::fmaxf(std::fmaxf(color.r, color.g), color.b);
		float radius =
		    (-light.getLinear() + std::sqrtf(light.getLinear() * light.getLinear() - 4 * light.getQuadratic() * (light.getConstant() - (256.0 / 5.0) * lightMax))) / (2 * light.getQuadratic());

		switch (light.getType()) {
			case LightType::LIGHT_POINT:
				{

					bx::Sphere lightPosRadius;
					lightPosRadius.center.x = pos[0];
					lightPosRadius.center.y = pos[1];
					lightPosRadius.center.z = pos[2];
					lightPosRadius.radius = radius;

					bx::toAabb(aabb, lightPosRadius);
					return aabb;
				}
			case LightType::LIGHT_DIR:
				{
					bx::Cone lightPos;
					lightPos.pos.x = pos[0];
					lightPos.pos.y = pos[1];
					lightPos.pos.z = pos[2];
					lightPos.radius = radius;
					// lightPos.end.x

					return aabb;
				}
			case LightType::LIGHT_SPOT:
				{
					return aabb;
				}
			default:
			case LightType::LIGHT_UNKNOWN:
				throw std::runtime_error("[RawrBox-G-BUFFER] Unknown light type");
		}
	}

	void G_BUFFER::lightPass(const rawrbox::Vector2i& size) {
		if (rawrbox::LIGHTS::fullbright) return;

		const auto width = static_cast<float>(size.x);
		const auto height = static_cast<float>(size.y);

		auto viewproj_inv = rawrbox::MAIN_CAMERA->getProjViewMtx();
		auto view = rawrbox::MAIN_CAMERA->getViewMtx();
		viewproj_inv.inverse();

		// Setup view
		rawrbox::CURRENT_VIEW_ID = rawrbox::GBUFFER_LIGHT_VIEW_ID;
		bgfx::setViewRect(rawrbox::CURRENT_VIEW_ID, 0, 0, size.x, size.y);
		bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, _lightBuffer->getBuffer());

		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, _orthoProj.data()); // Todo, move it to light for shadow calculation?

		// Build light data ----
		size_t lightCount = rawrbox::LIGHTS::count();
		for (size_t i = 0; i < lightCount; i++) {
			auto& light = rawrbox::LIGHTS::getLight(i);
			if (!light.isOn()) continue;
			if (i > 0) continue; // test code

			auto pos = light.getPosMatrix();
			auto& color = light.getSpecularColor();
			bx::Aabb aabb = calculateAABB(light);

			const bx::Vec3 box[8] =
			    {
				{aabb.min.x, aabb.min.y, aabb.min.z},
				{aabb.min.x, aabb.min.y, aabb.max.z},
				{aabb.min.x, aabb.max.y, aabb.min.z},
				{aabb.min.x, aabb.max.y, aabb.max.z},
				{aabb.max.x, aabb.min.y, aabb.min.z},
				{aabb.max.x, aabb.min.y, aabb.max.z},
				{aabb.max.x, aabb.max.y, aabb.min.z},
				{aabb.max.x, aabb.max.y, aabb.max.z},
			    };

			bx::Vec3 xyz = bx::mulH(box[0], view.data());
			bx::Vec3 min = xyz;
			bx::Vec3 max = xyz;

			for (uint32_t ii = 1; ii < 8; ++ii) {
				xyz = bx::mulH(box[ii], view.data());
				min = bx::min(min, xyz);
				max = bx::max(max, xyz);
			}

			if (max.z < 0.0F) {
				continue; // Outside of camera, hide it
			}

			const float x0 = bx::clamp((min.x * 0.5F + 0.5F) * width, 0.0F, width);
			const float y0 = bx::clamp((min.y * 0.5F + 0.5F) * height, 0.0F, height);
			const float x1 = bx::clamp((max.x * 0.5F + 0.5F) * width, 0.0F, width);
			const float y1 = bx::clamp((max.y * 0.5F + 0.5F) * height, 0.0F, height);

			const auto scissorHeight = uint16_t(y1 - y0);
			bgfx::setScissor(uint16_t(x0), uint16_t(height - scissorHeight - y0), uint16_t(x1 - x0), uint16_t(scissorHeight));
			// bgfx::setUniform(u_lightPosition, &lightPosRadius);
			bgfx::setUniform(u_lightData, light.getDataMatrix().data());

			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ADD);

			bgfx::setTexture(0, _s_normal, _gbufferTex[GBUFFER_RT_NORMAL]);
			bgfx::setTexture(1, _s_depth, _gbufferTex[GBUFFER_RT_DEPTH]);

			rawrbox::RenderUtils::renderScreenQuad(size);
			bgfx::submit(rawrbox::CURRENT_VIEW_ID, _programLight);
		}
		// -----------
	}

	void G_BUFFER::combine(const rawrbox::Vector2i& size) {
		rawrbox::CURRENT_VIEW_ID = rawrbox::GBUFFER_COMBINE_VIEW_ID;
		bgfx::setViewRect(rawrbox::CURRENT_VIEW_ID, 0, 0, size.x, size.y);
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, _orthoProj.data());
		bgfx::setViewFrameBuffer(rawrbox::CURRENT_VIEW_ID, BGFX_INVALID_HANDLE);

		bgfx::setTexture(0, _s_albedo, _gbufferTex[GBUFFER_RT_ALBEDO]);
		bgfx::setTexture(1, _s_light, _lightBuffer->getHandle());
		bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);

		rawrbox::RenderUtils::renderScreenQuad(size);
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, _programCombine);
	}

	void G_BUFFER::render(const rawrbox::Vector2i& size) {
		bgfx::ViewId oldview = rawrbox::CURRENT_VIEW_ID;

		// Light pass
		lightPass(size);
		// -----

		// Combine
		combine(size);
		// -----

		rawrbox::CURRENT_VIEW_ID = oldview;
	}

	void G_BUFFER::shutdown() {
		RAWRBOX_DESTROY(_gbuffer);

		RAWRBOX_DESTROY(_programCombine);
		RAWRBOX_DESTROY(_programLight);

		RAWRBOX_DESTROY(_s_albedo);
		RAWRBOX_DESTROY(_s_normal);
		RAWRBOX_DESTROY(_s_depth);

		RAWRBOX_DESTROY(u_lightPosition);
		RAWRBOX_DESTROY(u_lightData);

		for (auto i : _gbufferTex)
			RAWRBOX_DESTROY(i);

		_lightBuffer.reset();
	}

} // namespace rawrbox
