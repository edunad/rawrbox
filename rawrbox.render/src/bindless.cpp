
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PROTECTED ---
	std::vector<Diligent::IDeviceObject*> BindlessManager::_textureHandles = {};
	std::vector<Diligent::IDeviceObject*> BindlessManager::_vertexTextureHandles = {};

	std::vector<rawrbox::TextureBase*> BindlessManager::_updateTextures = {};

	std::vector<Diligent::StateTransitionDesc> BindlessManager::_barriers = {};
	// --------------

	// PUBLIC -------
	Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> BindlessManager::signature;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> BindlessManager::signatureBind;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferPixel;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> BindlessManager::signatureBufferVertex;
	// --------------

	void BindlessManager::init() {
		if (signature != nullptr) throw std::runtime_error("[RawrBox-BindlessManager] Signature already bound!");

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

		    {Diligent::SHADER_TYPE_VERTEX, "g_Textures", renderer->MAX_VERTEX_TEXTURES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
		    {Diligent::SHADER_TYPE_VERTEX, "g_Textures_sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},

		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures", renderer->MAX_TEXTURES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures_sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
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
		PRSDesc.UseCombinedTextureSamplers = true;
		// --------------

		// SAMPLERS -----
		Diligent::SamplerDesc SamLinearClampDesc{
		    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
		    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

		std::vector<Diligent::ImmutableSamplerDesc> samplers = {
		    {Diligent::SHADER_TYPE_VERTEX, "g_Textures", SamLinearClampDesc},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures", SamLinearClampDesc},
		};

		PRSDesc.ImmutableSamplers = samplers.data();
		PRSDesc.NumImmutableSamplers = static_cast<uint32_t>(samplers.size());
		// --------------

		// Buffer ------
		Diligent::BufferDesc BuffVertexDesc;
		BuffVertexDesc.Name = "rawrbox::SIGNATURE::Vertex::Uniforms";
		BuffVertexDesc.Usage = Diligent::USAGE_DYNAMIC;
		BuffVertexDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		BuffVertexDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		BuffVertexDesc.Size = sizeof(rawrbox::BindlessVertexBuffer);

		device->CreateBuffer(BuffVertexDesc, nullptr, &signatureBufferVertex);

		Diligent::BufferDesc BuffPixelDesc;
		BuffPixelDesc.Name = "rawrbox::SIGNATURE::Pixel::Uniforms";
		BuffPixelDesc.Usage = Diligent::USAGE_DYNAMIC;
		BuffPixelDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		BuffPixelDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		BuffPixelDesc.Size = sizeof(rawrbox::BindlessPixelBuffer);

		device->CreateBuffer(BuffPixelDesc, nullptr, &signatureBufferPixel);
		// -------------

		// Barrier ----
		rawrbox::BindlessManager::barrier(*signatureBufferVertex, rawrbox::BufferType::VERTEX);
		rawrbox::BindlessManager::barrier(*signatureBufferPixel, rawrbox::BufferType::INDEX);
		// ------------

		rawrbox::RENDERER->device()->CreatePipelineResourceSignature(PRSDesc, &signature);

		// Setup binds ---
		if (camera != nullptr) {
			signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Camera")->Set(rawrbox::RENDERER->camera()->uniforms());
			signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Camera")->Set(rawrbox::RENDERER->camera()->uniforms());
		}

		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(signatureBufferVertex);
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Constants")->Set(signatureBufferPixel);
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

		RAWRBOX_DESTROY(signature);
		RAWRBOX_DESTROY(signatureBind);
		RAWRBOX_DESTROY(signatureBufferPixel);
		RAWRBOX_DESTROY(signatureBufferVertex);
	}

	void BindlessManager::processBarriers() {
		if (_barriers.empty()) return;

		rawrbox::RENDERER->context()->TransitionResourceStates(static_cast<uint32_t>(_barriers.size()), _barriers.data());
		_barriers.clear();
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
	void BindlessManager::barrier(const rawrbox::TextureBase& texture) {
		auto texHandle = texture.getTexture();
		if (texHandle == nullptr) throw std::runtime_error(fmt::format("[RawrBox-BindlessManager] Texture '{}' not uploaded! Cannot create barrier", texture.getName()));

		_barriers.emplace_back(texture.getTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
	}

	void BindlessManager::barrier(Diligent::ITexture& texture, Diligent::RESOURCE_STATE state) {
		_barriers.emplace_back(&texture, Diligent::RESOURCE_STATE_UNKNOWN, state, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
	}

	void BindlessManager::barrier(Diligent::IBuffer& buffer, rawrbox::BufferType type) {
		_barriers.emplace_back(&buffer, Diligent::RESOURCE_STATE_UNKNOWN, mapResource(type), Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
	}

	void BindlessManager::immediateBarrier(Diligent::ITexture& texture, Diligent::RESOURCE_STATE state) {
		Diligent::StateTransitionDesc barrier = {&texture, Diligent::RESOURCE_STATE_UNKNOWN, state, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE};
		rawrbox::RENDERER->context()->TransitionResourceStates(1, &barrier);
	}

	void BindlessManager::immediateBarrier(Diligent::IBuffer& buffer, rawrbox::BufferType type) {
		Diligent::StateTransitionDesc barrier = {&buffer, Diligent::RESOURCE_STATE_UNKNOWN, mapResource(type), Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE};
		rawrbox::RENDERER->context()->TransitionResourceStates(1, &barrier);
	}
	// ----------------

	// REGISTER TEXTURES -------
	uint32_t BindlessManager::registerTexture(rawrbox::TextureBase& texture) {
		if (signature == nullptr) throw std::runtime_error("[RawrBox-BindlessManager] Signature not bound! Did you call init?");

		auto* pTextureSRV = texture.getHandle(); // Get shader resource view from the texture
		if (pTextureSRV == nullptr) throw std::runtime_error(fmt::format("[RawrBox-BindlessManager] Failed to register texture '{}'! Texture view is null, not uploaded?", texture.getName()));

		bool isVertex = texture.getType() == rawrbox::TEXTURE_TYPE::VERTEX;
		auto& handler = isVertex ? _vertexTextureHandles : _textureHandles;

		// Check if it's already registered --
		for (size_t slot = 0; slot < handler.size(); slot++) {
			if (handler[slot] == pTextureSRV) {
				return static_cast<uint32_t>(slot);
			}
		}
		//-------------------------

		// First find a empty slot -----------
		for (size_t slot = 0; slot < handler.size(); slot++) {
			if (handler[slot] == nullptr) {
				handler[slot] = pTextureSRV;

				// Register texture for updates --
				registerUpdateTexture(texture);
				// ---------

				fmt::print("[RawrBox-BindlessManager] Re-using slot '{}' for bindless {} texture '{}'\n", slot, isVertex ? "vertex" : "pixel", texture.getName());
				return static_cast<uint32_t>(slot);
			}
		}
		// ----------------------------

		// No slot ---
		auto slot = static_cast<uint32_t>(handler.size());

		if (slot == static_cast<uint32_t>(rawrbox::RENDERER->MAX_TEXTURES / 1.2F)) fmt::print("[RawrBox-BindlessManager] Aproaching max texture limit of '{}'\n", rawrbox::RENDERER->MAX_TEXTURES);
		if (slot >= rawrbox::RENDERER->MAX_TEXTURES) throw std::runtime_error(fmt::format("[RawrBox-BindlessManager] Max texture limit reached! Cannot allocate texture, remove some unecessary textures or increase MAX_TEXTURES on renderer\n", rawrbox::RENDERER->MAX_TEXTURES));
		handler.push_back(pTextureSRV);

		fmt::print("[RawrBox-BindlessManager] Registering bindless {} texture '{}' to slot '{}'\n", isVertex ? "vertex" : "pixel", texture.getName(), slot);
		// -----

		// Register texture for updates --
		registerUpdateTexture(texture);
		// ---------

		// Update signature ---
		if (signatureBind != nullptr) {
			signatureBind->GetVariableByName(isVertex ? Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(handler.data(), 0, static_cast<uint32_t>(handler.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}
		// -----

		return slot;
	}

	void BindlessManager::unregisterTexture(rawrbox::TextureBase& texture) {
		if (signature == nullptr) return;

		bool isVertex = texture.getType() == rawrbox::TEXTURE_TYPE::VERTEX;
		auto& handler = isVertex ? _vertexTextureHandles : _textureHandles;

		uint32_t id = texture.getTextureID();
		if (id >= handler.size()) throw std::runtime_error(fmt::format("[RawrBox-TextureManager] Index '{}' not found!", id));

		// Cleanup  ----
		unregisterUpdateTexture(texture);
		handler[id] = nullptr;
		// --------------------

		fmt::print("[RawrBox-TextureManager] Un-registering bindless {} texture slot '{}'\n", isVertex ? "vertex" : "pixel", id);

		// Update signature ---
		if (signatureBind != nullptr) {
			signatureBind->GetVariableByName(isVertex ? Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(handler.data(), 0, static_cast<uint32_t>(handler.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}
		// -----
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

		if (state == Diligent::RESOURCE_STATE_UNKNOWN) throw std::runtime_error("[RawrBox-BindlessManager] Invalid buffer type! Cannot create barrier");
		return state;
	}
	// --------------
}; // namespace rawrbox
