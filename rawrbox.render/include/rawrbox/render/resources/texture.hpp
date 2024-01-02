#pragma once

#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceTexture : public rawrbox::Resource {
		std::unique_ptr<rawrbox::TextureBase> _texture = nullptr;

	public:
		ResourceTexture() = default;
		ResourceTexture(const ResourceTexture&) = delete;
		ResourceTexture(ResourceTexture&&) = delete;
		ResourceTexture& operator=(const ResourceTexture&) = delete;
		ResourceTexture& operator=(ResourceTexture&&) = delete;
		~ResourceTexture() override;

		template <typename T = rawrbox::TextureBase>
		[[nodiscard]] T* get() const {
			return dynamic_cast<T*>(this->_texture.get());
		}

		bool load(const std::vector<uint8_t>& buffer) override;
		void upload() override;
	};

	class TextureLoader : public rawrbox::Loader {
	public:
		TextureLoader() = default;
		TextureLoader(const TextureLoader&) = delete;
		TextureLoader(TextureLoader&&) = delete;
		TextureLoader& operator=(const TextureLoader&) = delete;
		TextureLoader& operator=(TextureLoader&&) = delete;
		~TextureLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry() override;

		bool canLoad(const std::string& fileExtention) override;
		bool supportsBuffer() override;
	};
} // namespace rawrbox
