#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <DynamicBuffer.hpp>

#include <memory>
#include <vector>

namespace rawrbox {
	struct Decal {
		rawrbox::Matrix4x4 localToWorld = {};
		rawrbox::Colorf color = {};

		uint32_t textureID = 0;
		uint32_t textureAtlasIndex = 0;

		void setTexture(const rawrbox::TextureBase& texture, uint32_t id = 0) {
			if (!texture.isValid()) throw std::runtime_error("[RawrBox-DECAL] Invalid texture, not uploaded?");
			this->textureID = texture.getTextureID();
			this->textureAtlasIndex = id;
		}

		Decal() = default;
		Decal(const rawrbox::Matrix4x4& _mtx, const rawrbox::TextureBase& _texture, const rawrbox::Colorf& _color, uint32_t _atlas = 0) : localToWorld(_mtx), color(_color), textureAtlasIndex(_atlas) {
			this->setTexture(_texture);
		}
	};

	struct DecalConstants {
		rawrbox::Vector4_t<uint32_t> settings = {};
	};

	struct DecalVertex {
		rawrbox::Matrix4x4 worldToLocal = {};
		rawrbox::Vector4_t<uint32_t> data = {};
		rawrbox::Colorf color = {};
	};

	class DECALS {
	protected:
		static std::vector<std::unique_ptr<rawrbox::Decal>> _decals;

		static std::unique_ptr<Diligent::DynamicBuffer> _buffer;
		static Diligent::IBufferView* _bufferRead;

		static void update();

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		static void init();
		static void shutdown();

		static void bindUniforms();

		// UTILS ----
		static Diligent::IBufferView* getBuffer();

		static rawrbox::Decal* getDecal(size_t indx);
		static size_t count();
		// ----------

		// DECALS ----
		static void add(rawrbox::Decal decal);

		static bool remove(size_t indx);
		static bool remove(rawrbox::Decal* light);
		// ---------
	};
} // namespace rawrbox
