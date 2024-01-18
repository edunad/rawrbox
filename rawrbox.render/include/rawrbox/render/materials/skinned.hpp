#pragma once

#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {
	class MaterialSkinned : public rawrbox::MaterialUnlit {
		static bool _built;

	public:
		using vertexBufferType = rawrbox::VertexBoneData;

		MaterialSkinned() = default;
		MaterialSkinned(MaterialSkinned&&) = delete;
		MaterialSkinned& operator=(MaterialSkinned&&) = delete;
		MaterialSkinned(const MaterialSkinned&) = delete;
		MaterialSkinned& operator=(const MaterialSkinned&) = delete;
		~MaterialSkinned() override = default;

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
