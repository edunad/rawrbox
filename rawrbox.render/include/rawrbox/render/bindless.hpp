#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/render.hpp>

#include <RefCntAutoPtr.hpp>

#include <ShaderResourceBinding.h>

namespace rawrbox {
	struct BindlessVertexBuffer {
		// MODEL ---
		rawrbox::Vector4u data = {0xFFFFFFFF, 0, 0x00000000, 0}; // Color, Atlas, GPUId, Billboard
		rawrbox::Vector4f dataF = {};                            // VertexSnap, DisplacementTexture, DisplacementPower, ???
									 // ----------

		bool operator==(const BindlessVertexBuffer& other) const { return this->data == other.data && this->dataF == other.dataF; }
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
		rawrbox::Vector4u textureIDs = {}; // BASE, NORMAL, ROUGHTMETAL, EMISSION
		rawrbox::Vector4f litData = {};    // Texture data

		bool operator==(const BindlessPixelBuffer& other) const { return this->textureIDs == other.textureIDs && this->litData == other.litData; }
		bool operator!=(const BindlessPixelBuffer& other) const { return !operator==(other); }
	};

	struct BindlessPostProcessBuffer {
		std::array<rawrbox::Vector4f, rawrbox::MAX_POST_DATA> data = {};
		rawrbox::Vector4u textureIDs = {}; // BASE, DEPTH, ?, ?

		bool operator==(const BindlessPostProcessBuffer& other) const { return this->textureIDs == other.textureIDs && std::equal(this->data.begin(), this->data.end(), other.data.begin()); }
		bool operator!=(const BindlessPostProcessBuffer& other) const { return !operator==(other); }
	};
	// --------------------------

	class BindlessManager {
	protected:
		static std::vector<Diligent::IDeviceObject*> _textureHandles;
		static std::vector<Diligent::IDeviceObject*> _vertexTextureHandles;

		static std::vector<rawrbox::TextureBase*> _updateTextures;

		static bool _updateVertexSignature;
		static bool _updatePixelSignature;

		static std::unique_ptr<rawrbox::Logger> _logger;

		// SIGNATURES ------
		static void createSignatures();
		static void bindSignatures();
		// -----------------

		static uint32_t internalRegister(Diligent::ITextureView* view, rawrbox::TEXTURE_TYPE type);

	public:
		static Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> signature;
		static Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> signatureBind;

		static Diligent::RefCntAutoPtr<Diligent::IBuffer> signatureBufferPixel;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> signatureBufferVertex;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> signatureBufferVertexSkinned;

		static void init();
		static void shutdown();

		static void update();

		// TEXTURES -------
		static void registerTexture(rawrbox::TextureBase& texture);
		static void registerTexture(rawrbox::TextureRender& texture);

		static void registerUpdateTexture(rawrbox::TextureBase& texture);
		static void unregisterUpdateTexture(rawrbox::TextureBase& texture);

		static void unregisterTexture(rawrbox::TextureBase& texture);
		// ----------------
	};
} // namespace rawrbox
