#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/render.hpp>

#include <RefCntAutoPtr.hpp>

#include <ShaderResourceBinding.h>

namespace rawrbox {
	struct BindlessVertexBuffer {
		// MODEL ---
		rawrbox::Colorf colorOverride = {};
		rawrbox::Vector4f data = {};
		// ----------

		bool operator==(const BindlessVertexBuffer& other) const { return this->colorOverride == other.colorOverride && this->data == other.data; }
		bool operator!=(const BindlessVertexBuffer& other) const { return !operator==(other); }
	};

	struct BindlessVertexSkinnedBuffer {
		// MODEL BONES ----
		std::array<rawrbox::Matrix4x4, rawrbox::MAX_BONES_PER_MODEL> bones = {}; // This is quite heavy on the dynamic buffer
											 //-----------------

		bool operator==(const BindlessVertexSkinnedBuffer& other) const { return std::equal(this->bones.begin(), this->bones.end(), other.bones.begin()); }
		bool operator!=(const BindlessVertexSkinnedBuffer& other) const { return !operator==(other); }
	};
	// --------------------------

	struct BindlessPixelBuffer {
		rawrbox::Vector4_t<uint32_t> textureIDs = {}; // BASE, NORMAL, ROUGHTMETAL, EMISSION
		rawrbox::Vector4f litData = {};               // Texture data

		bool operator==(const BindlessPixelBuffer& other) const { return this->textureIDs == other.textureIDs && this->litData == other.litData; }
		bool operator!=(const BindlessPixelBuffer& other) const { return !operator==(other); }
	};

	struct BindlessPostProcessBuffer {
		std::array<rawrbox::Vector4f, rawrbox::MAX_POST_DATA> data = {};

		uint32_t textureID = 0;
		uint32_t depthTextureID = 0;

		bool operator==(const BindlessPostProcessBuffer& other) const { return this->textureID == other.textureID && this->depthTextureID == other.depthTextureID && std::equal(this->data.begin(), this->data.end(), other.data.begin()); }
		bool operator!=(const BindlessPostProcessBuffer& other) const { return !operator==(other); }
	};
	// --------------------------

	class BindlessManager {
	protected:
		static std::vector<Diligent::IDeviceObject*> _textureHandles;
		static std::vector<Diligent::IDeviceObject*> _vertexTextureHandles;

		static std::vector<rawrbox::TextureBase*> _updateTextures;

		// BARRIER -------------
		static std::unordered_map<Diligent::IDeviceObject*, Diligent::StateTransitionDesc> _barrierQueue;
		static std::unordered_map<Diligent::IDeviceObject*, Diligent::RESOURCE_STATE> _barrierCache;
		static std::vector<std::function<void()>> _barrierCallback;
		// -------------------------

		static std::unique_ptr<rawrbox::Logger> _logger;

		// SIGNATURES ------
		static void createSignatures();
		static void bindSignatures();
		// -----------------

		static uint32_t internalRegister(Diligent::ITextureView* view, rawrbox::TEXTURE_TYPE type);

	public:
		static Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> signature;
		static Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> computeSignature;
		static Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> signatureBind;
		static Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> computeSignatureBind;

		static Diligent::RefCntAutoPtr<Diligent::IBuffer> signatureBufferPixel;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> signatureBufferVertex;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> signatureBufferVertexSkinned;

		static void init();
		static void shutdown();

		static void update();

		// BARRIERS -------

		template <typename T>
			requires(std::derived_from<T, Diligent::IDeviceObject>)
		static void barrier(const std::vector<T*>& resources, const std::vector<Diligent::RESOURCE_STATE>& states, const std::function<void()>& callback = nullptr) {
			if (resources.size() != states.size()) throw _logger->error("Barrier resources and states do not match! {}-{}", resources.size(), states.size());

			for (size_t i = 0; i < resources.size(); i++) {
				_barrierQueue[resources[i]] = Diligent::StateTransitionDesc(resources[i], Diligent::RESOURCE_STATE_UNKNOWN, states[i], Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			}

			if (callback != nullptr) _barrierCallback.push_back(callback);
			if (std::this_thread::get_id() == rawrbox::RENDER_THREAD_ID) processBarriers();
		}

		static void processBarriers();
		static void clearBarrierCache();
		// ----------------

		// TEXTURES -------
		static void registerTexture(rawrbox::TextureBase& texture);
		static void registerTexture(rawrbox::TextureRender& texture);

		static void registerUpdateTexture(rawrbox::TextureBase& texture);
		static void unregisterUpdateTexture(rawrbox::TextureBase& texture);

		static void unregisterTexture(rawrbox::TextureBase& texture);
		// ----------------
	};
} // namespace rawrbox
