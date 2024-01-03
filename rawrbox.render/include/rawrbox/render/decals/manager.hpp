#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <DynamicBuffer.hpp>

#include <memory>
#include <vector>

namespace rawrbox {
	struct Decal {
		rawrbox::Matrix4x4 localToWorld = {};
		uint32_t textureIndex = 0;
		rawrbox::Colorf color = {};
	};

	struct DecalConstants {
		rawrbox::Vector4_t<uint32_t> g_DecalSettings = {};
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

		static rawrbox::TextureBase* _textureAtlas;

		static void update();

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		static void init();
		static void shutdown();

		static void bindUniforms();

		// ATLAS ----
		static void setAtlas(rawrbox::TextureBase* texture);
		static rawrbox::TextureBase* getAtlas();
		//-------------

		// UTILS ----
		static Diligent::IBufferView* getBuffer();

		static rawrbox::Decal* getDecal(size_t indx);
		static size_t count();
		// ----------

		// DECALS ----
		static rawrbox::Decal* add(const rawrbox::Matrix4x4& mtx, uint32_t atlasId, const rawrbox::Colorf& color = rawrbox::Colors::White());
		static bool remove(size_t indx);
		static bool remove(rawrbox::Decal* light);
		// ---------
	};
} // namespace rawrbox
