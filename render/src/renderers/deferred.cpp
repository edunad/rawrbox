
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/renderers/deferred.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader world_light[] = {
    BGFX_EMBEDDED_SHADER(vs_world_light),
    BGFX_EMBEDDED_SHADER(fs_world_light),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader point_light[] = {
    BGFX_EMBEDDED_SHADER(vs_light_base),
    BGFX_EMBEDDED_SHADER(fs_point_light),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

constexpr bgfx::ViewId GEOMETRY_VIEW_ID = 0;
constexpr bgfx::ViewId WORLD_LIGHT_VIEW_ID = 1;
constexpr bgfx::ViewId LIGHT_VIEW_ID = 2;
constexpr bgfx::ViewId DECAL_VIEW_ID = 3;
constexpr bgfx::ViewId FINAL_VIEW_ID = 4;

namespace rawrbox {
	RendererDeferred::~RendererDeferred() {
		rawrbox::LIGHTS::shutdown(); // Shutdown light system

		RAWRBOX_DESTROY(this->_gbuffer);
		RAWRBOX_DESTROY(this->_gbuffer_acc);

		RAWRBOX_DESTROY(this->_accTexture);
		RAWRBOX_DESTROY(this->_depthCopy);

		RAWRBOX_DESTROY(this->_worldLightProgram);
	}

	// -------------------------------------------
	void RendererDeferred::init(const rawrbox::Vector2i& size) {
		RendererBase::init(size);

		this->_gBufferSamplers = {
		    bgfx::createUniform("s_texDiffuse", bgfx::UniformType::Sampler),
		    bgfx::createUniform("s_texNormal", bgfx::UniformType::Sampler),
		    bgfx::createUniform("s_texEmissionSpec", bgfx::UniformType::Sampler),
		    bgfx::createUniform("s_texBitMask", bgfx::UniformType::Sampler),
		    bgfx::createUniform("s_texDepth", bgfx::UniformType::Sampler)};

		// Setup views ---
		bgfx::setViewName(GEOMETRY_VIEW_ID, "RAWRBOX-G_BUFFER-GEOMETRY");
		bgfx::setViewName(WORLD_LIGHT_VIEW_ID, "RAWRBOX-G_BUFFER-WORLD-LIGHT");
		bgfx::setViewName(LIGHT_VIEW_ID, "RAWRBOX-G_BUFFER-LIGHT");
		bgfx::setViewName(DECAL_VIEW_ID, "RAWRBOX-G_BUFFER-DECAL");
		bgfx::setViewName(FINAL_VIEW_ID, "RAWRBOX-G_BUFFER-FINAL");
		// ----

		// Load shaders --
		rawrbox::RenderUtils::buildShader(world_light, this->_worldLightProgram);
		rawrbox::RenderUtils::buildShader(point_light, this->_pointLightProgram);
		// ---

		// axis-aligned bounding box used as light geometry for light culling
		constexpr float LEFT = -1.0F, RIGHT = 1.0F, BOTTOM = -1.0F, TOP = 1.0F, FRONT = -1.0F, BACK = 1.0F;

		const std::array<rawrbox::PosVertexData, 8> vertices = {
		    PosVertexData(rawrbox::Vector3f(LEFT, BOTTOM, FRONT)),
		    PosVertexData(rawrbox::Vector3f(RIGHT, BOTTOM, FRONT)),
		    PosVertexData(rawrbox::Vector3f(LEFT, TOP, FRONT)),
		    PosVertexData(rawrbox::Vector3f(RIGHT, TOP, FRONT)),
		    PosVertexData(rawrbox::Vector3f(LEFT, BOTTOM, BACK)),
		    PosVertexData(rawrbox::Vector3f(RIGHT, BOTTOM, BACK)),
		    PosVertexData(rawrbox::Vector3f(LEFT, TOP, BACK)),
		    PosVertexData(rawrbox::Vector3f(RIGHT, TOP, BACK)),
		};

		const std::array<uint16_t, 36> indices = {
		    // CCW
		    0, 1, 3, 3, 2, 0, // front
		    5, 4, 6, 6, 7, 5, // back
		    4, 0, 2, 2, 6, 4, // left
		    1, 5, 7, 7, 3, 1, // right
		    2, 3, 7, 7, 6, 2, // top
		    4, 5, 1, 1, 0, 4  // bottom
		};

		this->_bbox_vb = bgfx::createVertexBuffer(bgfx::copy(&vertices, sizeof(vertices)), PosVertexData::vLayout());
		this->_bbox_ib = bgfx::createIndexBuffer(bgfx::copy(&indices, sizeof(indices)));

		this->_light_index = bgfx::createUniform("u_lightIndexVec", bgfx::UniformType::Vec4);

		// Initialize light engine
		rawrbox::LIGHTS::init();
	}

	void RendererDeferred::resize(const rawrbox::Vector2i& size) {
		RendererBase::resize(size);

		// Rebuild buffer -----
		RAWRBOX_DESTROY(this->_gbuffer);
		RAWRBOX_DESTROY(this->_gbuffer_acc);

		RAWRBOX_DESTROY(this->_accTexture);
		RAWRBOX_DESTROY(this->_depthCopy);

		// Regen textures ----
		const uint64_t flags = BGFX_TEXTURE_RT | rawrbox::RendererDeferred::GBUFFER_FLAGS;
		const auto depthFormat = bgfx::TextureFormat::D24;

		std::array<bgfx::TextureHandle, rawrbox::G_BUFFER::COUNT> textures = {
		    bgfx::createTexture2D(size.x, size.y, false, 1, bgfx::TextureFormat::BGRA8, flags), // DIFFUSE
		    bgfx::createTexture2D(size.x, size.y, false, 1, bgfx::TextureFormat::RG16F, flags), // NORMAL
		    bgfx::createTexture2D(size.x, size.y, false, 1, bgfx::TextureFormat::BGRA8, flags), // EMISSION_SPECULAR

		    bgfx::createTexture2D(size.x, size.y, false, 1, bgfx::TextureFormat::R8, flags), // BITMASK

		    bgfx::createTexture2D(size.x, size.y, false, 1, depthFormat, flags)}; // DEPTH

		this->_gbuffer = bgfx::createFrameBuffer(textures.size(), textures.data(), true);
		bgfx::setName(this->_gbuffer, "G-BUFFER");

		// Setup acc buffer ----
		this->_accTexture = bgfx::createTexture2D(size.x, size.y, false, 1, bgfx::TextureFormat::BGRA8, flags);

		const std::array<bgfx::TextureHandle, 2> accTextures = {this->_accTexture,
		    bgfx::getTexture(this->_gbuffer, G_BUFFER::DEPTH)};

		this->_gbuffer_acc = bgfx::createFrameBuffer(accTextures.size(), accTextures.data(), false);
		bgfx::setName(this->_gbuffer_acc, "G-BUFFER-ACCUMULATOR");
		// ----

		// Create blit copy ---
		this->_depthCopy = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, depthFormat, BGFX_TEXTURE_BLIT_DST | flags);
		// ----

		// Setup views ---
		bgfx::setViewClear(GEOMETRY_VIEW_ID, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0F);
		bgfx::setViewRect(GEOMETRY_VIEW_ID, 0, 0, size.x, size.y);
		bgfx::setViewFrameBuffer(GEOMETRY_VIEW_ID, this->_gbuffer);
		bgfx::touch(GEOMETRY_VIEW_ID);

		bgfx::setViewClear(WORLD_LIGHT_VIEW_ID, BGFX_CLEAR_COLOR, 0x00000000, 1.0F);
		bgfx::setViewRect(WORLD_LIGHT_VIEW_ID, 0, 0, size.x, size.y);
		bgfx::setViewFrameBuffer(WORLD_LIGHT_VIEW_ID, this->_gbuffer_acc);
		bgfx::touch(WORLD_LIGHT_VIEW_ID);

		bgfx::setViewClear(LIGHT_VIEW_ID, BGFX_CLEAR_NONE);
		bgfx::setViewRect(LIGHT_VIEW_ID, 0, 0, size.x, size.y);
		bgfx::setViewFrameBuffer(LIGHT_VIEW_ID, this->_gbuffer_acc);
		bgfx::touch(LIGHT_VIEW_ID);
		// -----
	}

	void RendererDeferred::render() {
		if (this->worldRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] World render method not set! Did you call 'setWorldRender' ?");
		if (this->overlayRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Overlay render method not set! Did you call 'setOverlayRender' ?");

		// No world / overlay only
		if (rawrbox::MAIN_CAMERA == nullptr) {
			rawrbox::RendererBase::render();
			return;
		}

		// Setup camera ---
		auto view = rawrbox::MAIN_CAMERA->getViewMtx();
		auto proj = rawrbox::MAIN_CAMERA->getProjMtx();

		bgfx::setViewTransform(GEOMETRY_VIEW_ID, view.data(), proj.data());
		bgfx::setViewTransform(LIGHT_VIEW_ID, view.data(), proj.data());
		bgfx::setViewTransform(DECAL_VIEW_ID, view.data(), proj.data());
		bgfx::setViewTransform(FINAL_VIEW_ID, view.data(), proj.data());
		bgfx::setViewTransform(WORLD_LIGHT_VIEW_ID, nullptr, nullptr);
		// ------

		// Populate G-BUFFER
		auto prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = GEOMETRY_VIEW_ID;
		// ---

		bgfx::touch(GEOMETRY_VIEW_ID);
		this->worldRender();
		// ---

		// Restore id -----
		rawrbox::CURRENT_VIEW_ID = prevId;
		bgfx::discard(BGFX_DISCARD_ALL);
		// ------------------------

		// Render world light ---
		prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = WORLD_LIGHT_VIEW_ID;
		bgfx::touch(WORLD_LIGHT_VIEW_ID);
		// ---

		// Copy depth ---
		bgfx::blit(WORLD_LIGHT_VIEW_ID, this->_depthCopy, 0, 0, bgfx::getTexture(this->_gbuffer, rawrbox::G_BUFFER::DEPTH));
		// -------

		this->bindRenderUniforms();
		rawrbox::LIGHTS::bindUniforms();

		rawrbox::RenderUtils::drawQUAD(this->_size, false, BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_GREATER | BGFX_STATE_CULL_CW);
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_worldLightProgram, 0, ~BGFX_DISCARD_BINDINGS);
		// ---

		// Restore id -----
		rawrbox::CURRENT_VIEW_ID = prevId;
		bgfx::discard(BGFX_DISCARD_ALL);
		// ------------------------

		// Render lights ---
		prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = LIGHT_VIEW_ID;
		bgfx::touch(LIGHT_VIEW_ID);
		// ---

		// Setup light BBOX ---
		bgfx::setVertexBuffer(0, this->_bbox_vb);
		bgfx::setIndexBuffer(this->_bbox_ib);
		// -----

		// Lights ----
		this->bindRenderUniforms();
		this->bindCamera();
		rawrbox::LIGHTS::bindUniforms();
		for (size_t i = 0; i < rawrbox::LIGHTS::count(); i++) {
			this->applyLight(i);
		}
		// ------

		// ---------------------

		this->finalRender();
		this->frame(); // Submit ---
	}

	void RendererDeferred::applyLight(size_t indx) {
		auto light = rawrbox::LIGHTS::getLight(indx);
		if (light == nullptr || light->getType() != rawrbox::LightType::LIGHT_POINT) return; // TODO: SPOT LIGHT

		float radius = light->getRadius();

		rawrbox::Matrix4x4 mtx = {};
		mtx.mtxSRT({radius, radius, radius}, {}, light->getPos());

		bgfx::setTransform(mtx.data());

		std::array<float, 4> lightIndexVec = {static_cast<float>(indx)};
		bgfx::setUniform(this->_light_index, lightIndexVec.data());
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_GEQUAL | BGFX_STATE_CULL_CCW |
			       BGFX_STATE_BLEND_ADD);

		bgfx::submit(rawrbox::CURRENT_VIEW_ID,
		    this->_pointLightProgram,
		    0,
		    ~(BGFX_DISCARD_VERTEX_STREAMS | BGFX_DISCARD_INDEX_BUFFER | BGFX_DISCARD_BINDINGS));
	}

	void RendererDeferred::finalRender() {
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_WORLD_VIEW;
		bgfx::touch(CURRENT_VIEW_ID);

		rawrbox::RenderUtils::drawQUAD(this->_accTexture, this->_size);
		bgfx::discard();
	}

	void RendererDeferred::bindRenderUniforms() {
		if (!bgfx::isValid(this->_gbuffer)) throw std::runtime_error("[Rawrbox-Renderer] G-BUFFER not set! Did you call 'init' ?");

		bgfx::setTexture(rawrbox::DEFERRED_DIFFUSE, this->_gBufferSamplers[rawrbox::G_BUFFER::DIFFUSE], bgfx::getTexture(this->_gbuffer, rawrbox::G_BUFFER::DIFFUSE));
		bgfx::setTexture(rawrbox::DEFERRED_NORMAL, this->_gBufferSamplers[rawrbox::G_BUFFER::NORMAL], bgfx::getTexture(this->_gbuffer, rawrbox::G_BUFFER::NORMAL));
		bgfx::setTexture(rawrbox::DEFERRED_EMISSION_SPEC, this->_gBufferSamplers[rawrbox::G_BUFFER::EMISSION_SPECULAR], bgfx::getTexture(this->_gbuffer, rawrbox::G_BUFFER::EMISSION_SPECULAR));
		bgfx::setTexture(rawrbox::DEFERRED_BITMASK, this->_gBufferSamplers[rawrbox::G_BUFFER::BITMASK], bgfx::getTexture(this->_gbuffer, rawrbox::G_BUFFER::BITMASK));
		bgfx::setTexture(rawrbox::DEFERRED_DEPTH, this->_gBufferSamplers[rawrbox::G_BUFFER::DEPTH], this->_depthCopy);
	}

	bool RendererDeferred::supported() {
		const bgfx::Caps* caps = bgfx::getCaps();

		return RendererBase::supported() &&
		       (caps->supported & BGFX_CAPS_TEXTURE_BLIT) != 0 &&
		       caps->limits.maxFBAttachments >= G_BUFFER::COUNT - 1; // Depth doesn't count as attachment
	}
} // namespace rawrbox
