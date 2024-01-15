
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PROTECTED ---
	std::vector<Diligent::IDeviceObject*> BindlessManager::_textureHandles = {};
	std::vector<Diligent::IDeviceObject*> BindlessManager::_vertexTextureHandles = {};

	std::vector<rawrbox::TextureBase*> BindlessManager::_updateTextures = {};

	std::vector<Diligent::StateTransitionDesc> BindlessManager::_barriers = {};
	std::vector<std::function<void()>> BindlessManager::_barriersCallbacks = {};

	std::unique_ptr<rawrbox::Logger> BindlessManager::_logger = std::make_unique<rawrbox::Logger>("RawrBox-BindlessManager");
	// --------------

	// PUBLIC -------
	Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> BindlessManager::signature;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> BindlessManager::signatureBind;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferPixel;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferVertex;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferPostProcess;
	// --------------

	void BindlessManager::init() {
		if (signature != nullptr) throw _logger->error("Signature already bound!");
		_logger->info("Initializing bindless manager");

		auto renderer = rawrbox::RENDERER;
		_textureHandles.reserve(renderer->MAX_TEXTURES);
		_vertexTextureHandles.reserve(renderer->MAX_VERTEX_TEXTURES);

		// Create signature -----
		auto camera = renderer->camera();
		auto context = renderer->context();
		auto device = renderer->device();

		Diligent::PipelineResourceSignatureDesc PRSDesc;
		PRSDesc.Name = "RawrBox::SIGNATURE::BINDLESS";
		PRSDesc.BindingIndex = 0;

		// RESOURCES -----
		std::vector<Diligent::PipelineResourceDesc> resources = {
		    {Diligent::SHADER_TYPE_VERTEX, "Constants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "Constants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_PIXEL, "PostProcessConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_VERTEX, "g_Textures", renderer->MAX_VERTEX_TEXTURES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
		    //{Diligent::SHADER_TYPE_VERTEX, "g_Sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures", renderer->MAX_TEXTURES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
		    //{Diligent::SHADER_TYPE_PIXEL, "g_Sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		};

		if (camera != nullptr) {
			resources.emplace_back(Diligent::SHADER_TYPE_VERTEX, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
			resources.emplace_back(Diligent::SHADER_TYPE_PIXEL, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
		}

		// Add extra signatures ----
		for (auto& plugin : renderer->getPlugins()) {
			if (plugin.second == nullptr) continue;
			plugin.second->signatures(resources);
		}
		// -------------------------

		PRSDesc.Resources = resources.data();
		PRSDesc.NumResources = static_cast<uint8_t>(resources.size());
		// --------------

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

		// Buffer ------
		{
			Diligent::BufferDesc BuffVertexDesc;
			BuffVertexDesc.Name = "rawrbox::SIGNATURE::Vertex::Uniforms";
			BuffVertexDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffVertexDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffVertexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffVertexDesc.Size = sizeof(rawrbox::BindlessVertexBuffer);

			device->CreateBuffer(BuffVertexDesc, nullptr, &signatureBufferVertex);
			rawrbox::BindlessManager::barrier(*signatureBufferVertex, rawrbox::BufferType::CONSTANT);
		}

		{
			Diligent::BufferDesc BuffPixelDesc;
			BuffPixelDesc.Name = "rawrbox::SIGNATURE::Pixel::Uniforms";
			BuffPixelDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffPixelDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffPixelDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffPixelDesc.Size = sizeof(rawrbox::BindlessPixelBuffer);

			device->CreateBuffer(BuffPixelDesc, nullptr, &signatureBufferPixel);
			rawrbox::BindlessManager::barrier(*signatureBufferPixel, rawrbox::BufferType::CONSTANT);
		}

		{
			Diligent::BufferDesc BuffPixelDesc;
			BuffPixelDesc.Name = "rawrbox::SIGNATURE::PostProcess";
			BuffPixelDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffPixelDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffPixelDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffPixelDesc.Size = sizeof(rawrbox::BindlessPostProcessBuffer);

			device->CreateBuffer(BuffPixelDesc, nullptr, &signatureBufferPostProcess);
			rawrbox::BindlessManager::barrier(*signatureBufferPostProcess, rawrbox::BufferType::CONSTANT);
		}
		// -------------

		// Barrier ----
		// ------------

		rawrbox::RENDERER->device()->CreatePipelineResourceSignature(PRSDesc, &signature);

		// Setup binds ---
		if (camera != nullptr) {
			signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Camera")->Set(rawrbox::RENDERER->camera()->uniforms());
			signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Camera")->Set(rawrbox::RENDERER->camera()->uniforms());
		}

		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(signatureBufferVertex);
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(signatureBufferPixel);

		// signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Sampler")->Set(rawrbox::PipelineUtils::defaultSampler);
		// signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Sampler")->Set(rawrbox::PipelineUtils::defaultSampler);

		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "PostProcessConstants")->Set(signatureBufferPostProcess);
		// ----------------

		// Add extra signatures ----
		for (auto& plugin : rawrbox::RENDERER->getPlugins()) {
			if (plugin.second == nullptr) continue;
			plugin.second->bind(*signature);
		}
		// -------------------------

		signature->CreateShaderResourceBinding(&signatureBind, true);

		// Setup textures ---
		signatureBind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Textures")->SetArray(_vertexTextureHandles.data(), 0, static_cast<uint32_t>(_vertexTextureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(_textureHandles.data(), 0, static_cast<uint32_t>(_textureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		// ----------------------
	}

	void BindlessManager::shutdown() {
		_textureHandles = {};
		_vertexTextureHandles = {};
		_updateTextures = {};

		_barriers = {};
		_barriersCallbacks = {};

		RAWRBOX_DESTROY(signature);
		RAWRBOX_DESTROY(signatureBind);
		RAWRBOX_DESTROY(signatureBufferPixel);
		RAWRBOX_DESTROY(signatureBufferVertex);
	}

	void BindlessManager::processBarriers() {
		if (_barriers.empty()) return;

		rawrbox::RENDERER->context()->TransitionResourceStates(static_cast<uint32_t>(_barriers.size()), _barriers.data());
		for (auto& callback : _barriersCallbacks) {
			callback();
		}

		_barriers.clear();
		_barriersCallbacks.clear();
	}

	void BindlessManager::registerUpdateTexture(rawrbox::TextureBase& tex) {
		if (!tex.requiresUpdate()) return;
		_updateTextures.push_back(&tex);
	}

	void BindlessManager::unregisterUpdateTexture(rawrbox::TextureBase& tex) {
		auto fnd = std::find(_updateTextures.begin(), _updateTextures.end(), &tex);
		if (fnd == _updateTextures.end()) return;

		_updateTextures.erase(fnd);
	}

	void BindlessManager::update() {
		if (signature == nullptr) return;

		// UPDATE TEXTURES ---
		if (!_updateTextures.empty()) {
			for (auto tex : _updateTextures) {
				if (tex == nullptr) continue;

				tex->update();
				barrier(*tex); // Maybe?
			}
		}
		// ---------------------

		// PROCESS GPU BARRIER ---
		processBarriers();
		// ---------------
	}

	// BARRIERS -------
	void BindlessManager::barrier(const rawrbox::TextureBase& texture, std::function<void()> callback) {
		auto texHandle = texture.getTexture();
		if (texHandle == nullptr) throw _logger->error("Texture '{}' not uploaded! Cannot create barrier", texture.getName());

		auto threadID = std::this_thread::get_id();
		if (threadID != rawrbox::RENDER_THREAD_ID) { // Context is not thread safe, so we need to queue it
			_barriers.emplace_back(texture.getTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			if (callback != nullptr) _barriersCallbacks.push_back(callback);
		} else {
			Diligent::StateTransitionDesc barrier = {texHandle, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE};
			rawrbox::RENDERER->context()->TransitionResourceStates(1, &barrier);

			if (callback != nullptr) callback();
		}
	}

	void BindlessManager::barrier(Diligent::ITexture& texture, Diligent::RESOURCE_STATE state, std::function<void()> callback) {
		auto threadID = std::this_thread::get_id();
		if (threadID != rawrbox::RENDER_THREAD_ID) { // Context is not thread safe, so we need to queue it
			_barriers.emplace_back(&texture, Diligent::RESOURCE_STATE_UNKNOWN, state, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			if (callback != nullptr) _barriersCallbacks.push_back(callback);
		} else {
			Diligent::StateTransitionDesc barrier = {&texture, Diligent::RESOURCE_STATE_UNKNOWN, state, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE};
			rawrbox::RENDERER->context()->TransitionResourceStates(1, &barrier);

			if (callback != nullptr) callback();
		}
	}

	void BindlessManager::barrier(Diligent::IBuffer& buffer, rawrbox::BufferType type, std::function<void()> callback) {
		auto threadID = std::this_thread::get_id();
		if (threadID != rawrbox::RENDER_THREAD_ID) { // Context is not thread safe, so we need to queue it
			_barriers.emplace_back(&buffer, Diligent::RESOURCE_STATE_UNKNOWN, mapResource(type), Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			if (callback != nullptr) _barriersCallbacks.push_back(callback);
		} else {
			Diligent::StateTransitionDesc barrier = {&buffer, Diligent::RESOURCE_STATE_UNKNOWN, mapResource(type), Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE};
			rawrbox::RENDERER->context()->TransitionResourceStates(1, &barrier);

			if (callback != nullptr) callback();
		}
	}
	// ----------------

	// REGISTER TEXTURES -------
	void BindlessManager::registerTexture(rawrbox::TextureRender& texture) {
		if (signature == nullptr) throw _logger->error("Signature not bound! Did you call init?");
		if (texture.isRegistered()) return; // Check if it's already registered --

		auto* pDepthSRV = texture.getDepth(); // Get depth
		if (pDepthSRV != nullptr) {
			uint32_t id = internalRegister(pDepthSRV, rawrbox::TEXTURE_TYPE::PIXEL);
			_logger->info("Registering {} bindless pixel texture slot '{}'", fmt::format(fmt::fg(fmt::color::red), "DEPTH"), fmt::format(fmt::fg(fmt::color::violet), std::to_string(id)));

			// Register depth
			texture.setDepthTextureID(id);
			//----
		}

		// Ok, now do normal
		registerTexture(dynamic_cast<rawrbox::TextureBase&>(texture)); // Normal register, horrible i know.
	}

	void BindlessManager::registerTexture(rawrbox::TextureBase& texture) {
		if (signature == nullptr) throw _logger->error("Signature not bound! Did you call init?");
		if (texture.isRegistered()) return; // Check if it's already registered --

		auto* pTextureSRV = texture.getHandle(); // Get shader resource view from the texture
		if (pTextureSRV == nullptr) throw _logger->error("Failed to register texture '{}'! Texture view is null, not uploaded?", texture.getName());

		uint32_t id = internalRegister(pTextureSRV, texture.getType());
		_logger->info("Registering bindless {} texture slot '{}'", texture.getType() == rawrbox::TEXTURE_TYPE::VERTEX ? "vertex" : "pixel", fmt::format(fmt::fg(fmt::color::violet), std::to_string(id)));

		// ----
		registerUpdateTexture(texture);
		texture.setTextureID(id);
		// ------------
	}

	void BindlessManager::unregisterTexture(rawrbox::TextureBase& texture) {
		if (signature == nullptr) return;

		bool isVertex = texture.getType() == rawrbox::TEXTURE_TYPE::VERTEX;
		auto& handler = isVertex ? _vertexTextureHandles : _textureHandles;

		auto id = texture.getTextureID();
		auto depthId = texture.getDepthTextureID();

		if (id >= handler.size()) throw _logger->error("Index '{}' not found!", id);
		if (depthId >= handler.size()) throw _logger->error("Depth index '{}' not found!", id);

		// Cleanup  ----
		unregisterUpdateTexture(texture);
		handler[id] = nullptr;

		if (depthId != 0) handler[depthId] = nullptr;
		// --------------------

		_logger->info("Un-registering bindless {} texture slot '{}'", isVertex ? "vertex" : "pixel", fmt::format(fmt::fg(fmt::color::violet), std::to_string(id)));

		// Update signature ---
		if (signatureBind != nullptr) {
			signatureBind->GetVariableByName(isVertex ? Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(handler.data(), 0, static_cast<uint32_t>(handler.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}
		// -----
	}

	uint32_t BindlessManager::internalRegister(Diligent::ITextureView* view, rawrbox::TEXTURE_TYPE type) {
		bool isVertex = type == rawrbox::TEXTURE_TYPE::VERTEX;
		uint32_t max = isVertex ? rawrbox::RENDERER->MAX_VERTEX_TEXTURES : rawrbox::RENDERER->MAX_TEXTURES;
		auto& handler = isVertex ? _vertexTextureHandles : _textureHandles;

		int size = static_cast<int>(handler.size());
		if (size == (max / 1.2F)) _logger->warn("Aproaching max texture limit of {}", fmt::format(fmt::fg(fmt::color::red), std::to_string(max)));
		if (size >= max) throw _logger->error("Max texture limit reached! Cannot allocate texture, remove some unecessary textures or increase max textures on renderer");

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

		// Update signature ---
		if (signatureBind != nullptr) {
			signatureBind->GetVariableByName(isVertex ? Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(handler.data(), 0, static_cast<uint32_t>(handler.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}
		// -----

		return id;
	}

	Diligent::RESOURCE_STATE BindlessManager::mapResource(rawrbox::BufferType type) {
		auto state = Diligent::RESOURCE_STATE_UNKNOWN;
		switch (type) {
			case BufferType::CONSTANT:
				state = Diligent::RESOURCE_STATE_CONSTANT_BUFFER;
				break;
			case BufferType::INDEX:
				state = Diligent::RESOURCE_STATE_INDEX_BUFFER;
				break;
			case BufferType::VERTEX:
				state = Diligent::RESOURCE_STATE_VERTEX_BUFFER;
				break;
			case BufferType::UNORDERED_ACCESS:
				state = Diligent::RESOURCE_STATE_UNORDERED_ACCESS;
				break;
			case BufferType::SHADER:
				state = Diligent::RESOURCE_STATE_SHADER_RESOURCE;
				break;
		}

		if (state == Diligent::RESOURCE_STATE_UNKNOWN) throw _logger->error("Invalid buffer type! Cannot create barrier");
		return state;
	}
	// --------------
}; // namespace rawrbox
