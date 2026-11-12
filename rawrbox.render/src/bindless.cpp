
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/uniform_cache.hpp>

#include <MapHelper.hpp>
#include <tracy/Tracy.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PROTECTED ---
	std::vector<Diligent::IDeviceObject*> BindlessManager::_textureHandles = {};
	std::vector<Diligent::IDeviceObject*> BindlessManager::_vertexTextureHandles = {};

	std::vector<rawrbox::TextureBase*> BindlessManager::_updateTextures = {};

	bool BindlessManager::_updatePixelSignature = false;
	bool BindlessManager::_updateVertexSignature = false;

	std::unique_ptr<rawrbox::Logger> BindlessManager::_logger = std::make_unique<rawrbox::Logger>("RawrBox-BindlessManager");
	// --------------

	// PUBLIC -------
	Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> BindlessManager::signature;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> BindlessManager::signatureBind;

	Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> BindlessManager::instanceSignature;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferPixel;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferVertex;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferVertexSkinned;
	// --------------

	void BindlessManager::init() {
		if (BindlessManager::signature != nullptr) RAWRBOX_CRITICAL("Signature already bound!");
		BindlessManager::_logger->info("Initializing bindless manager");

		auto* renderer = rawrbox::RENDERER;
		auto* device = renderer->device();

		// Reserve max textures ---
		BindlessManager::_textureHandles.reserve(RB_RENDER_MAX_TEXTURES);
		BindlessManager::_vertexTextureHandles.reserve(RB_RENDER_MAX_VERTEX_TEXTURES);
		// ------------------------

		// Create Buffer ------
		{
			Diligent::BufferDesc BuffVertexDesc;
			BuffVertexDesc.Name = "rawrbox::SIGNATURE::Vertex::Uniforms";
			BuffVertexDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffVertexDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffVertexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffVertexDesc.Size = sizeof(rawrbox::BindlessVertexBuffer);

			device->CreateBuffer(BuffVertexDesc, nullptr, &BindlessManager::signatureBufferVertex);
		}

		{
			Diligent::BufferDesc BuffPixelDesc;
			BuffPixelDesc.Name = "rawrbox::SIGNATURE::Vertex:::Skinned::Uniforms";
			BuffPixelDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffPixelDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffPixelDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffPixelDesc.Size = sizeof(rawrbox::BindlessVertexSkinnedBuffer);

			device->CreateBuffer(BuffPixelDesc, nullptr, &BindlessManager::signatureBufferVertexSkinned);

			// Horrible temp fix for signatures that might never be mapped ---
			Diligent::MapHelper<rawrbox::BindlessVertexSkinnedBuffer> HACK(rawrbox::RENDERER->context(), BindlessManager::signatureBufferVertexSkinned, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			// -----------
		}

		{
			Diligent::BufferDesc BuffPixelDesc;
			BuffPixelDesc.Name = "rawrbox::SIGNATURE::Pixel::Uniforms";
			BuffPixelDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffPixelDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffPixelDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffPixelDesc.Size = sizeof(rawrbox::BindlessPixelBuffer);

			device->CreateBuffer(BuffPixelDesc, nullptr, &BindlessManager::signatureBufferPixel);
		}
		// -------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{BindlessManager::signatureBufferVertexSkinned, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {BindlessManager::signatureBufferPixel, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------

		// Create signatures ------
		BindlessManager::createSignatures();
		// ------------
	}

	void BindlessManager::shutdown() {
		BindlessManager::_textureHandles = {};
		BindlessManager::_vertexTextureHandles = {};
		BindlessManager::_updateTextures = {};

		BindlessManager::_updateVertexSignature = false;
		BindlessManager::_updatePixelSignature = false;

		RAWRBOX_DESTROY(BindlessManager::signature);
		RAWRBOX_DESTROY(BindlessManager::signatureBind);

		rawrbox::UniformCache::clear();
		RAWRBOX_DESTROY(BindlessManager::instanceSignature);

		RAWRBOX_DESTROY(BindlessManager::signatureBufferPixel);
		RAWRBOX_DESTROY(BindlessManager::signatureBufferVertex);
		RAWRBOX_DESTROY(BindlessManager::signatureBufferVertexSkinned);
	}

	// SIGNATURES ---------
	void BindlessManager::createSignatures() {
		auto* renderer = rawrbox::RENDERER;
		auto* device = renderer->device();

		Diligent::PipelineResourceSignatureDesc PRSDesc;
		PRSDesc.Name = "RawrBox::SIGNATURE::BINDLESS";
		PRSDesc.BindingIndex = 0;

		// Graphics signature ---
		std::vector<Diligent::PipelineResourceDesc> resources = {
		    {Diligent::SHADER_TYPE_VERTEX, "Constants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_VERTEX, "SkinnedConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_VERTEX, "g_Textures", RB_RENDER_MAX_VERTEX_TEXTURES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},

		    {Diligent::SHADER_TYPE_PIXEL, "Constants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures", RB_RENDER_MAX_TEXTURES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},

		    {Diligent::SHADER_TYPE_VERTEX | Diligent::SHADER_TYPE_PIXEL | Diligent::SHADER_TYPE_GEOMETRY, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_VERTEX | Diligent::SHADER_TYPE_PIXEL | Diligent::SHADER_TYPE_GEOMETRY, "SCamera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS}};

		// Add extra signatures ----
		for (const auto& plugin : renderer->getPlugins()) {
			if (plugin.second == nullptr) continue;
			plugin.second->signatures(resources);
		}
		// -------------------------
		PRSDesc.Resources = resources.data();
		PRSDesc.NumResources = static_cast<uint8_t>(resources.size());

		// SAMPLERS -----
		Diligent::SamplerDesc SamLinearClampDesc{
		    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
		    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

		std::vector<Diligent::ImmutableSamplerDesc> samplers = {
		    {Diligent::SHADER_TYPE_VERTEX, "g_Sampler", SamLinearClampDesc},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Sampler", SamLinearClampDesc},
		};

		PRSDesc.ImmutableSamplers = samplers.data();
		PRSDesc.NumImmutableSamplers = static_cast<uint32_t>(samplers.size());
		// --------------

		device->CreatePipelineResourceSignature(PRSDesc, &BindlessManager::signature);
		// ----------------------

		// Instanced ---
		constexpr Diligent::PipelineResourceDesc instances = {
		    Diligent::SHADER_TYPE_VERTEX, "Instances", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS};

		Diligent::PipelineResourceSignatureDesc instanceDesc;
		instanceDesc.Name = "RawrBox::SIGNATURE::INSTANCES";
		instanceDesc.BindingIndex = 2;
		instanceDesc.Resources = &instances;
		instanceDesc.NumResources = 1;

		device->CreatePipelineResourceSignature(instanceDesc, &BindlessManager::instanceSignature);
		if (BindlessManager::instanceSignature == nullptr) RAWRBOX_CRITICAL("Failed to create instance signature");

		const uint32_t contexts = rawrbox::RENDERER->threadPool()->contextCount();
		if (contexts > RB_RENDER_MAX_CONTEXTS) RAWRBOX_CRITICAL("Too many render contexts ({}), max is {}", contexts, RB_RENDER_MAX_CONTEXTS);
		// ----------------------
	}

	void BindlessManager::bindSignatures() {
		// Setup graphic binds ---
		BindlessManager::signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(BindlessManager::signatureBufferVertex);
		BindlessManager::signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "SkinnedConstants")->Set(BindlessManager::signatureBufferVertexSkinned);

		BindlessManager::signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Camera")->Set(rawrbox::CameraBase::uniforms);
		BindlessManager::signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Camera")->Set(rawrbox::CameraBase::uniforms);
		BindlessManager::signature->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "Camera")->Set(rawrbox::CameraBase::uniforms);

		BindlessManager::signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);
		BindlessManager::signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);
		BindlessManager::signature->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);

		BindlessManager::signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(BindlessManager::signatureBufferPixel);
		// ------------
		// Add extra binds ----
		for (const auto& plugin : rawrbox::RENDERER->getPlugins()) {
			if (plugin.second == nullptr) continue;
			plugin.second->bindStatic(*BindlessManager::signature);
		}
		// -------------------------

		BindlessManager::signature->CreateShaderResourceBinding(&BindlessManager::signatureBind, true);

		// Bind signature bind ----
		for (const auto& plugin : rawrbox::RENDERER->getPlugins()) {
			if (plugin.second == nullptr) continue;
			plugin.second->bindMutable(*BindlessManager::signatureBind);
		}
		// -------------------------

		// Setup textures ---
		BindlessManager::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Textures")->SetArray(BindlessManager::_vertexTextureHandles.data(), 0, static_cast<uint32_t>(BindlessManager::_vertexTextureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		BindlessManager::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(BindlessManager::_textureHandles.data(), 0, static_cast<uint32_t>(BindlessManager::_textureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		// ----------------------
		// ----------------
	}
	// --------------------------

	void BindlessManager::bindContext(Diligent::IDeviceContext& context) {
		if (BindlessManager::signatureBind == nullptr) RAWRBOX_CRITICAL("Bindless manager not initialized!");

		rawrbox::UniformCache::reset();
		rawrbox::UniformCache::map(context, BindlessManager::signatureBufferVertex, rawrbox::BindlessVertexBuffer{});
		rawrbox::UniformCache::map(context, BindlessManager::signatureBufferVertexSkinned, rawrbox::BindlessVertexSkinnedBuffer{});
		rawrbox::UniformCache::map(context, BindlessManager::signatureBufferPixel, rawrbox::BindlessPixelBuffer{});

		context.CommitShaderResources(BindlessManager::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
	}

	void BindlessManager::update() {
		if (BindlessManager::signature == nullptr || BindlessManager::signatureBind == nullptr) return;
		ZoneScopedN("BindlessManager::update");

		// UPDATE TEXTURES ---
		for (size_t i = 0; i < BindlessManager::_updateTextures.size();) {
			auto* texture = BindlessManager::_updateTextures[i];
			if (texture == nullptr || !texture->requiresUpdate()) {
				BindlessManager::_updateTextures.erase(BindlessManager::_updateTextures.begin() + static_cast<std::ptrdiff_t>(i));
				continue;
			}

			texture->update();
			i++;
		}
		// ---------------------

		// Flush old ones
		if (BindlessManager::_updateVertexSignature || BindlessManager::_updatePixelSignature) rawrbox::RENDERER->context()->WaitForIdle();
		// ---------------------

		// UPDATE SHADER BIND ---
		if (BindlessManager::_updateVertexSignature) {
			BindlessManager::_updateVertexSignature = false;
			BindlessManager::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Textures")->SetArray(BindlessManager::_vertexTextureHandles.data(), 0, static_cast<uint32_t>(BindlessManager::_vertexTextureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}

		if (BindlessManager::_updatePixelSignature) {
			BindlessManager::_updatePixelSignature = false;
			BindlessManager::signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(BindlessManager::_textureHandles.data(), 0, static_cast<uint32_t>(BindlessManager::_textureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}
		// ---------------------
	}

	// REGISTER TEXTURES -------
	void BindlessManager::registerTexture(rawrbox::TextureRender& texture) {
		if (BindlessManager::signature == nullptr) RAWRBOX_CRITICAL("Signature not bound! Did you call init?");
		if (texture.isRegistered()) return; // Check if it's already registered --

		auto* pDepthSRV = texture.getDepth(); // Get depth
		if (pDepthSRV != nullptr) {
			auto id = BindlessManager::internalRegister(pDepthSRV, rawrbox::TEXTURE_TYPE::PIXEL);
			BindlessManager::_logger->debug("Registering {} bindless texture '{}' on slot '{}'", fmt::styled("DEPTH", fmt::fg(fmt::color::red)), fmt::styled(texture.getName(), fmt::fg(fmt::color::violet)), fmt::styled(std::to_string(id), fmt::fg(fmt::color::violet)));

			// Register depth
			texture.setDepthTextureID(id);
			//----
		}

		// Ok, now do normal
		BindlessManager::registerTexture(static_cast<rawrbox::TextureBase&>(texture));
	}

	void BindlessManager::registerTexture(rawrbox::TextureBase& texture) {
		if (BindlessManager::signature == nullptr) RAWRBOX_CRITICAL("Signature not bound! Did you call init?");
		if (texture.isRegistered()) return; // Check if it's already registered --

		auto* pTextureSRV = texture.getHandle(); // Get shader resource view from the texture
		if (pTextureSRV == nullptr) RAWRBOX_CRITICAL("Failed to register texture '{}'! Texture view is null, not uploaded?", texture.getName());

		const bool isVertex = texture.getType() == rawrbox::TEXTURE_TYPE::VERTEX;
		const auto id = BindlessManager::internalRegister(pTextureSRV, texture.getType());
		BindlessManager::_logger->debug("Registering bindless {} texture '{}' on slot '{}'", isVertex ? "vertex" : "pixel", fmt::styled(texture.getName(), fmt::fg(fmt::color::violet)), fmt::styled(std::to_string(id), fmt::fg(fmt::color::violet)));

		// Register for updates
		BindlessManager::registerUpdateTexture(texture);
		texture.setTextureID(id);
		// ------------
	}

	void BindlessManager::updateTextureHandle(rawrbox::TextureBase& texture) {
		if (BindlessManager::signature == nullptr) RAWRBOX_CRITICAL("Signature not bound! Did you call init?");
		if (!texture.isRegistered()) RAWRBOX_CRITICAL("Texture '{}' is not registered!", texture.getName());

		auto* view = texture.getHandle();
		if (view == nullptr) RAWRBOX_CRITICAL("Failed to update texture '{}'! Texture view is null, not uploaded?", texture.getName());

		const bool isVertex = texture.getType() == rawrbox::TEXTURE_TYPE::VERTEX;
		auto& handler = isVertex ? BindlessManager::_vertexTextureHandles : BindlessManager::_textureHandles;

		const auto id = texture.getTextureID();
		if (id >= handler.size()) RAWRBOX_CRITICAL("Index '{}' not found!", id);

		handler[id] = view;
		if (isVertex) {
			BindlessManager::_updateVertexSignature = true;
		} else {
			BindlessManager::_updatePixelSignature = true;
		}

		BindlessManager::_logger->debug("Updated bindless '{}' texture handle on slot '{}'", isVertex ? "vertex" : "pixel", fmt::styled(std::to_string(id), fmt::fg(fmt::color::violet)));
	}

	void BindlessManager::updateTextureHandle(rawrbox::TextureRender& texture) {
		auto* depth = texture.getDepth();
		const auto depthId = texture.getDepthTextureID();

		if (depth != nullptr && depthId != 0) {
			if (depthId >= BindlessManager::_textureHandles.size()) RAWRBOX_CRITICAL("Depth index '{}' not found!", depthId);

			BindlessManager::_textureHandles[depthId] = depth;
			BindlessManager::_updatePixelSignature = true;
		}

		BindlessManager::updateTextureHandle(static_cast<rawrbox::TextureBase&>(texture));
	}

	void BindlessManager::unregisterTexture(rawrbox::TextureBase& texture) {
		if (BindlessManager::signature == nullptr || !texture.isRegistered()) return;

		const bool isVertex = texture.getType() == rawrbox::TEXTURE_TYPE::VERTEX;
		auto& handler = isVertex ? BindlessManager::_vertexTextureHandles : BindlessManager::_textureHandles;

		const auto id = texture.getTextureID();
		const auto depthId = texture.getDepthTextureID();

		if (id >= handler.size()) RAWRBOX_CRITICAL("Index '{}' not found!", id);
		if (depthId >= handler.size()) RAWRBOX_CRITICAL("Depth index '{}' not found!", depthId);

		// Cleanup  ----
		BindlessManager::unregisterUpdateTexture(texture);
		handler[id] = nullptr;

		if (depthId != 0) handler[depthId] = nullptr;
		// --------------------

		// No need to update signature, it will be overriden by another texture
		BindlessManager::_logger->debug("Un-registering bindless {} texture slot '{}'", isVertex ? "vertex" : "pixel", fmt::styled(std::to_string(id), fmt::fg(fmt::color::violet)));
	}

	void BindlessManager::registerUpdateTexture(rawrbox::TextureBase& texture) {
		if (!texture.requiresUpdate() || std::ranges::contains(BindlessManager::_updateTextures, &texture)) return;
		BindlessManager::_updateTextures.push_back(&texture);
	}

	void BindlessManager::unregisterUpdateTexture(rawrbox::TextureBase& texture) {
		if (BindlessManager::_updateTextures.empty()) return;
		std::erase(BindlessManager::_updateTextures, &texture);
	}

	uint32_t BindlessManager::internalRegister(Diligent::ITextureView* view, rawrbox::TEXTURE_TYPE type) {
		const bool isVertex = type == rawrbox::TEXTURE_TYPE::VERTEX;
		const auto max = isVertex ? RB_RENDER_MAX_VERTEX_TEXTURES : RB_RENDER_MAX_TEXTURES;
		auto& handler = isVertex ? BindlessManager::_vertexTextureHandles : BindlessManager::_textureHandles;

		const int size = static_cast<int>(handler.size());
		if (size == static_cast<int>(max / 1.2F)) BindlessManager::_logger->warn("Aproaching max texture limit of {}", fmt::styled(std::to_string(max), fmt::fg(fmt::color::red)));
		if (size >= static_cast<int>(max)) RAWRBOX_CRITICAL("Max texture limit reached! Cannot allocate texture, remove some unecessary textures or increase max textures on renderer");

		int id = -1;

		// First find a empty slot -----------
		for (int i = 0; i < size; i++) {
			if (handler[i] == nullptr) {
				id = i;
				break;
			}
		}
		// ----------------------------

		// Register new slot -----------
		if (id == -1) {
			id = size;
			handler.push_back(view);
		} else {
			handler[id] = view;
		}

		// Trigger bind update ----
		if (isVertex) {
			BindlessManager::_updateVertexSignature = true;
		} else {
			BindlessManager::_updatePixelSignature = true;
		}
		// ------------

		return id;
	}
	// --------------
}; // namespace rawrbox