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
		bool bindVertexSkinnedUniforms(const rawrbox::Mesh<T>& mesh) {
			rawrbox::BindlessVertexSkinnedBuffer buff = this->bindBaseVertexSkinnedUniforms<T>(mesh);
			if (this->_lastSkinnedVertexBuffer.first && buff == this->_lastSkinnedVertexBuffer.second) return false;
			this->_lastSkinnedVertexBuffer = {true, buff};

			// SETUP UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::BindlessVertexSkinnedBuffer> VertexSkinnedConstants(rawrbox::RENDERER->context(), rawrbox::BindlessManager::signatureBufferVertexSkinned, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				std::memcpy(VertexSkinnedConstants, &this->_lastSkinnedVertexBuffer.second, sizeof(rawrbox::BindlessVertexSkinnedBuffer));
			}
			// -----------
			return true;
		}
	};

} // namespace rawrbox
