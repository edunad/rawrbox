
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PROTECTED ---
	std::vector<Diligent::IDeviceObject*> BindlessManager::_textureHandles = {};
	std::unordered_map<uint32_t, rawrbox::TextureBase*> BindlessManager::_updateTextures = {};

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

		// Create signature -----
		auto renderer = rawrbox::RENDERER;

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
		signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(_textureHandles.data(), 0, static_cast<uint32_t>(_textureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		// ----------------------
	}

	void BindlessManager::shutdown() {
		_textureHandles = {};
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

	void BindlessManager::update() {
		if (signature == nullptr) return;

		// UPDATE TEXTURES ---
		if (!_updateTextures.empty()) {
			for (auto tex : _updateTextures) {
				if (tex.second == nullptr) continue;

				tex.second->update();
				barrier(*tex.second); // Maybe?
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

	void BindlessManager::barrier(Diligent::IBuffer& buffer, rawrbox::BufferType type) {
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
		}

		if (state == Diligent::RESOURCE_STATE_UNKNOWN) throw std::runtime_error("[RawrBox-BindlessManager] Invalid buffer type! Cannot create barrier");
		_barriers.emplace_back(&buffer, Diligent::RESOURCE_STATE_UNKNOWN, state, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
	}
	// ----------------

	// REGISTER TEXTURES -------
	uint32_t BindlessManager::registerTexture(rawrbox::TextureBase& texture) {
		if (signature == nullptr) throw std::runtime_error("[RawrBox-BindlessManager] Signature not bound! Did you call init?");

		auto* pTextureSRV = texture.getHandle(); // Get shader resource view from the texture
		if (pTextureSRV == nullptr) throw std::runtime_error(fmt::format("[RawrBox-BindlessManager] Failed to register texture '{}'! Texture view is null, not uploaded?", texture.getName()));

		// Check if it's already registered --
		for (size_t slot = 0; slot < _textureHandles.size(); slot++) {
			if (_textureHandles[slot] == pTextureSRV) {
				return static_cast<uint32_t>(slot);
			}
		}
		//-------------------------

		// First find a empty slot -----------
		for (size_t slot = 0; slot < _textureHandles.size(); slot++) {
			if (_textureHandles[slot] == nullptr) {
				_textureHandles[slot] = pTextureSRV;
				if (texture.requiresUpdate()) _updateTextures[slot] = &texture;

				fmt::print("[RawrBox-BindlessManager] Re-using slot '{}' for bindless texture '{}'\n", slot, texture.getName());
				return static_cast<uint32_t>(slot);
			}
		}
		// ----------------------------

		// No slot ---
		auto slot = static_cast<uint32_t>(_textureHandles.size());
		if (slot == static_cast<uint32_t>(rawrbox::RENDERER->MAX_TEXTURES / 1.2F)) fmt::print("[RawrBox-BindlessManager] Aproaching max texture limit of '{}'\n", rawrbox::RENDERER->MAX_TEXTURES);
		if (slot >= rawrbox::RENDERER->MAX_TEXTURES) throw std::runtime_error(fmt::format("[RawrBox-BindlessManager] Max texture limit reached! Cannot allocate texture, remove some unecessary textures or increase MAX_TEXTURES on renderer\n", rawrbox::RENDERER->MAX_TEXTURES));
		_textureHandles.push_back(pTextureSRV);

		if (texture.requiresUpdate()) _updateTextures[slot] = &texture;
		fmt::print("[RawrBox-BindlessManager] Registering bindless texture '{}' to slot '{}'\n", texture.getName(), slot);
		// -----

		// Update signature ---
		if (signatureBind != nullptr) {
			signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(_textureHandles.data(), 0, static_cast<uint32_t>(_textureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}
		// -----

		return slot;
	}

	void BindlessManager::unregisterTexture(uint32_t indx) {
		if (signature == nullptr) return;

		if (indx >= _textureHandles.size()) throw std::runtime_error(fmt::format("[RawrBox-TextureManager] Index '{}' not found!", indx));
		_textureHandles[indx] = nullptr;

		// Cleanup update textures ----
		auto fnd = _updateTextures.find(indx);
		if (fnd != _updateTextures.end()) {
			_updateTextures.erase(indx);
		}
		// --------------------

		fmt::print("[RawrBox-TextureManager] Un-registering bindless texture slot '{}'\n", indx);

		// Update signature ---
		if (signatureBind != nullptr) {
			signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(_textureHandles.data(), 0, static_cast<uint32_t>(_textureHandles.size()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		}
		// -----
	}
	// --------------
}; // namespace rawrbox
