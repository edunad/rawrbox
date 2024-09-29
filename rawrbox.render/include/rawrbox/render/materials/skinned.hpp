#pragma once

#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {
	class MaterialSkinned : public rawrbox::MaterialUnlit {
		static bool _built;

	public:
#ifdef _DEBUG
		static bool DEBUG_MODE;
#endif

		using vertexBufferType = rawrbox::VertexBoneData;

		MaterialSkinned() = default;
		MaterialSkinned(MaterialSkinned&&) = delete;
		MaterialSkinned& operator=(MaterialSkinned&&) = delete;
		MaterialSkinned(const MaterialSkinned&) = delete;
		MaterialSkinned& operator=(const MaterialSkinned&) = delete;
		~MaterialSkinned() override = default;

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
				if (VertexSkinnedConstants == nullptr) throw _logger->error("Failed to map the vertex skinned constants buffer!");

				std::memcpy(VertexSkinnedConstants, &this->_lastSkinnedVertexBuffer.value(), sizeof(rawrbox::BindlessVertexSkinnedBuffer));
			}
			// -----------

			return true;
		}
	};

} // namespace rawrbox
