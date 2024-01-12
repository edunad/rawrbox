#pragma once

#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <RefCntAutoPtr.hpp>

#include <ShaderResourceBinding.h>

namespace rawrbox {
	constexpr auto MAX_DATA = 4;

	enum class BufferType {
		CONSTANT,
		INDEX,
		VERTEX
	};

	// --------------------------
	struct BindlessVertexBuffer {
		// UNLIT ---
		rawrbox::Colorf colorOverride = {};
		rawrbox::Vector4f textureFlags = {};

		std::array<rawrbox::Vector4f, MAX_DATA> data = {}; // Other mesh data, like vertex / displacement / billboard settings / masks
								   // ----------
	};
	// --------------------------

	struct BindlessPixelBuffer {
		rawrbox::Vector4_t<uint32_t> textureIDs = {}; // BASE, NORMAL, ROUGHTMETAL, EMISSION
		rawrbox::Vector4f litData = {};
	};

	// --------------------------

	class BindlessManager {
	protected:
		static std::vector<Diligent::IDeviceObject*> _textureHandles;
		static std::vector<Diligent::IDeviceObject*> _vertexTextureHandles;

		static std::vector<rawrbox::TextureBase*> _updateTextures;

		static std::vector<Diligent::StateTransitionDesc> _barriers;

		static void processBarriers();

		static void registerUpdateTexture(rawrbox::TextureBase& tex);
		static void unregisterUpdateTexture(rawrbox::TextureBase& tex);

	public:
		static Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> signature;
		static Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> signatureBind;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> signatureBufferPixel;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> signatureBufferVertex;

		static void init();
		static void shutdown();
		static void update();

		// BARRIERS -------
		static void barrier(const rawrbox::TextureBase& texture);
		static void barrier(Diligent::ITexture& texture, Diligent::RESOURCE_STATE state = Diligent::RESOURCE_STATE_SHADER_RESOURCE);
		static void barrier(Diligent::IBuffer& buffer, rawrbox::BufferType type);

		static void immediateBarrier(Diligent::ITexture& texture, Diligent::RESOURCE_STATE state = Diligent::RESOURCE_STATE_SHADER_RESOURCE);
		// ----------------

		// TEXTURES -------
		static uint32_t registerTexture(rawrbox::TextureBase& texture);
		static void unregisterTexture(rawrbox::TextureBase& texture);
		// ----------------
	};
} // namespace rawrbox
