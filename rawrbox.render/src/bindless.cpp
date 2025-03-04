
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>

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

	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferPixel;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferVertex;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferVertexSkinned;
	// --------------

	void BindlessManager::init() {
		if (signature != nullptr) RAWRBOX_CRITICAL("Signature already bound!");
		_logger->info("Initializing bindless manager");

		auto* renderer = rawrbox::RENDERER;
		auto* device = renderer->device();

		// Reserve max textures ---
		_textureHandles.reserve(RB_RENDER_MAX_TEXTURES);
		_vertexTextureHandles.reserve(RB_RENDER_MAX_VERTEX_TEXTURES);
		// ------------------------

		// Create Buffer ------
		{
			Diligent::BufferDesc BuffVertexDesc;
			BuffVertexDesc.Name = "rawrbox::SIGNATURE::Vertex::Uniforms";
			BuffVertexDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffVertexDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffVertexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffVertexDesc.Size = sizeof(rawrbox::BindlessVertexBuffer);

			device->CreateBuffer(BuffVertexDesc, nullptr, &signatureBufferVertex);
		}

		{
			Diligent::BufferDesc BuffPixelDesc;
			BuffPixelDesc.Name = "rawrbox::SIGNATURE::Vertex:::Skinned::Uniforms";
			BuffPixelDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffPixelDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffPixelDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffPixelDesc.Size = sizeof(rawrbox::BindlessVertexSkinnedBuffer);

			device->CreateBuffer(BuffPixelDesc, nullptr, &signatureBufferVertexSkinned);

			// Horrible temp fix for signatures that might never be mapped ---
			Diligent::MapHelper<rawrbox::BindlessVertexSkinnedBuffer> HACK(rawrbox::RENDERER->context(), signatureBufferVertexSkinned, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			// -----------
		}

		{
			Diligent::BufferDesc BuffPixelDesc;
			BuffPixelDesc.Name = "rawrbox::SIGNATURE::Pixel::Uniforms";
			BuffPixelDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffPixelDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffPixelDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffPixelDesc.Size = sizeof(rawrbox::BindlessPixelBuffer);

			device->CreateBuffer(BuffPixelDesc, nullptr, &signatureBufferPixel);
		}
		// -------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{signatureBufferVertexSkinned, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {signatureBufferPixel, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------

		// Create signatures ------
		createSignatures();
		// ------------
	}

	void BindlessManager::shutdown() {
		_textureHandles = {};
		_vertexTextureHandles = {};
		_updateTextures = {};

		_updateVertexSignature = false;
		_updatePixelSignature = false;

		RAWRBOX_DESTROY(signature);
		RAWRBOX_DESTROY(signatureBind);

		RAWRBOX_DESTROY(signatureBufferPixel);
		RAWRBOX_DESTROY(signatureBufferVertex);
		RAWRBOX_DESTROY(signatureBufferVertexSkinned);
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

		device->CreatePipelineResourceSignature(PRSDesc, &signature);
		// ----------------------
	}

	void BindlessManager::bindSignatures() {
		// Setup graphic binds ---
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(signatureBufferVertex);
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "SkinnedConstants")->Set(signatureBufferVertexSkinned);

		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Camera")->Set(rawrbox::CameraBase::uniforms);
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Camera")->Set(rawrbox::CameraBase::uniforms);
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "Camera")->Set(rawrbox::CameraBase::uniforms);

		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_GEOMETRY, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);

		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(signatureBufferPixel);
		// ------------
		// Add extra binds ----
		for (const auto& plugin : rawrbox::RENDERER->getPlugins()) {
			if (plugin.second == nullptr) continue;
			plugin.second->bindStatic(*signature);
		}
		// -------------------------

		signature->CreateShaderResourceBinding(&signatureBind, true);

		// Bind signature bind ----
		for (const auto& plugin : rawrbox::RENDERER->getPlugins()) {
			if (plugin.second == nullptr) continue;
			plugin.second->bindMutable(*signatureBind);
		}
		// -------------------------

		// Setup textures ---
		signatureBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Textures")->SetArray(_vertexTextureHandles.data(), 0, static_cast<uint32_t>(_vertexTextureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(_textureHandles.data(), 0, static_cast<uint32_t>(_textureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		// ----------------------
		// ----------------
	}
	// --------------------------

	void BindlessManager::update() {
		if (signature == nullptr || signatureBind == nullptr) return;

		// UPDATE TEXTURES ---
		if (!_updateTextures.empty()) {
			for (auto it = _updateTextures.begin(); it != _updateTextures.end();) {
				auto& texture = (*it);
				if (texture == nullptr || !texture->requiresUpdate()) {
					it = _updateTextures.erase(it);
					continue;
				}

				texture->update();
				++it;
			}
		}
		// ---------------------

		// UPDATE SHADER BIND ---
		if (_updateVertexSignature) {
			_updateVertexSignature = false;
			signatureBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Textures")->SetArray(_vertexTextureHandles.data(), 0, static_cast<uint32_t>(_vertexTextureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}

		if (_updatePixelSignature) {
			_updatePixelSignature = false;
			signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(_textureHandles.data(), 0, static_cast<uint32_t>(_textureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}
		// ---------------------
	}

	// REGISTER TEXTURES -------
	void BindlessManager::registerTexture(rawrbox::TextureRender& texture) {
		if (signature == nullptr) RAWRBOX_CRITICAL("Signature not bound! Did you call init?");
		if (texture.isRegistered()) return; // Check if it's already registered --

		auto* pDepthSRV = texture.getDepth(); // Get depth
		if (pDepthSRV != nullptr) {
			auto id = internalRegister(pDepthSRV, rawrbox::TEXTURE_TYPE::PIXEL);
			_logger->debug("Registering {} bindless texture '{}' on slot '{}'", fmt::styled("DEPTH", fmt::fg(fmt::color::red)), fmt::styled(texture.getName(), fmt::fg(fmt::color::violet)), fmt::styled(std::to_string(id), fmt::fg(fmt::color::violet)));

			// Register depth
			texture.setDepthTextureID(id);
			//----
		}

		// Ok, now do normal
		registerTexture(dynamic_cast<rawrbox::TextureBase&>(texture)); // Normal register, horrible i know.
	}

	void BindlessManager::registerTexture(rawrbox::TextureBase& texture) {
		if (signature == nullptr) RAWRBOX_CRITICAL("Signature not bound! Did you call init?");
		if (texture.isRegistered()) return; // Check if it's already registered --

		auto* pTextureSRV = texture.getHandle(); // Get shader resource view from the texture
		if (pTextureSRV == nullptr) RAWRBOX_CRITICAL("Failed to register texture '{}'! Texture view is null, not uploaded?", texture.getName());

		const bool isVertex = texture.getType() == rawrbox::TEXTURE_TYPE::VERTEX;
		const auto id = internalRegister(pTextureSRV, texture.getType());
		_logger->debug("Registering bindless {} texture '{}' on slot '{}'", isVertex ? "vertex" : "pixel", fmt::styled(texture.getName(), fmt::fg(fmt::color::violet)), fmt::styled(std::to_string(id), fmt::fg(fmt::color::violet)));

		// Register for updates
		registerUpdateTexture(texture);
		texture.setTextureID(id);
		// ------------
	}

	void BindlessManager::unregisterTexture(rawrbox::TextureBase& texture) {
		if (signature == nullptr) return;

		const bool isVertex = texture.getType() == rawrbox::TEXTURE_TYPE::VERTEX;
		auto& handler = isVertex ? _vertexTextureHandles : _textureHandles;

		const auto id = texture.getTextureID();
		const auto depthId = texture.getDepthTextureID();

		if (id >= handler.size()) RAWRBOX_CRITICAL("Index '{}' not found!", id);
		if (depthId >= handler.size()) RAWRBOX_CRITICAL("Depth index '{}' not found!", id);

		// Cleanup  ----
		unregisterUpdateTexture(texture);
		handler[id] = nullptr;

		if (depthId != 0) handler[depthId] = nullptr;
		// --------------------

		// No need to update signature, it will be overriden by another texture
		_logger->debug("Un-registering bindless {} texture slot '{}'", isVertex ? "vertex" : "pixel", fmt::styled(std::to_string(id), fmt::fg(fmt::color::violet)));
	}

	void BindlessManager::registerUpdateTexture(rawrbox::TextureBase& texture) {
		if (!texture.requiresUpdate()) return;
		_updateTextures.push_back(&texture);
	}

	void BindlessManager::unregisterUpdateTexture(rawrbox::TextureBase& texture) {
		auto fnd = std::find(_updateTextures.begin(), _updateTextures.end(), &texture);
		if (fnd == _updateTextures.end()) return;

		_updateTextures.erase(fnd);
	}

	uint32_t BindlessManager::internalRegister(Diligent::ITextureView* view, rawrbox::TEXTURE_TYPE type) {
		bool isVertex = type == rawrbox::TEXTURE_TYPE::VERTEX;
		auto max = isVertex ? RB_RENDER_MAX_VERTEX_TEXTURES : RB_RENDER_MAX_TEXTURES;
		auto& handler = isVertex ? _vertexTextureHandles : _textureHandles;

		int size = static_cast<int>(handler.size());
		if (size == static_cast<int>(max / 1.2F)) _logger->warn("Aproaching max texture limit of {}", fmt::styled(std::to_string(max), fmt::fg(fmt::color::red)));
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
			_updateVertexSignature = true;
		} else {
			_updatePixelSignature = true;
		}
		// ------------

		return id;
	}
	// --------------
}; // namespace rawrbox
