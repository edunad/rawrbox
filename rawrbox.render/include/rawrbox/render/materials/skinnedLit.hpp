#pragma once

#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>

namespace rawrbox {
	class MaterialSkinnedLit : public rawrbox::MaterialLit {
		static bool _built;

	public:
		using vertexBufferType = rawrbox::VertexNormBoneData;

		MaterialSkinnedLit() = default;
		MaterialSkinnedLit(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit& operator=(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit(const MaterialSkinnedLit&) = delete;
		MaterialSkinnedLit& operator=(const MaterialSkinnedLit&) = delete;
		~MaterialSkinnedLit() override = default;

		void init() override;

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		bool bindVertexSkinnedUniforms(const rawrbox::Mesh<T>& mesh) {
			rawrbox::BindlessVertexSkinnedBuffer buff = this->bindBaseVertexSkinnedUniforms<T>(mesh);
			if (this->_lastSkinnedVertexBuffer.has_value() && buff == this->_lastSkinnedVertexBuffer.value()) return false;
			this->_lastSkinnedVertexBuffer = buff;

			// SETUP UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::BindlessVertexSkinnedBuffer> VertexSkinnedConstants(rawrbox::RENDERER->context(), rawrbox::BindlessManager::signatureBufferVertexSkinned, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				if (VertexSkinnedConstants == nullptr) RAWRBOX_CRITICAL("Failed to map the vertex skinned constants buffer!");

				std::memcpy(VertexSkinnedConstants, &this->_lastSkinnedVertexBuffer.value(), sizeof(rawrbox::BindlessVertexSkinnedBuffer));
			}
			// -----------
			return true;
		}
	};

} // namespace rawrbox
