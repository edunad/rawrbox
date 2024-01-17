#pragma once

#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceWEBM : public rawrbox::Resource {
		std::unique_ptr<rawrbox::TextureBase> _texture = nullptr;

	public:
		ResourceWEBM() = default;
		ResourceWEBM(const ResourceWEBM&) = delete;
		ResourceWEBM(ResourceWEBM&&) = delete;
		ResourceWEBM& operator=(const ResourceWEBM&) = delete;
		ResourceWEBM& operator=(ResourceWEBM&&) = delete;
		~ResourceWEBM() override;

		template <typename T = rawrbox::TextureBase>
		[[nodiscard]] T* get() const {
			return dynamic_cast<T*>(this->_texture.get());
		}

		bool load(const std::vector<uint8_t>& buffer) override;
		void upload() override;
	};

	class WEBMLoader : public rawrbox::Loader {
	public:
		WEBMLoader();
		WEBMLoader(const WEBMLoader&) = delete;
		WEBMLoader(WEBMLoader&&) = delete;
		WEBMLoader& operator=(const WEBMLoader&) = delete;
		WEBMLoader& operator=(WEBMLoader&&) = delete;
		~WEBMLoader() override;

		std::unique_ptr<rawrbox::Resource> createEntry() override;
		bool canLoad(const std::string& fileExtention) override;
		bool supportsBuffer(const std::string& fileExtention) override;
	};
} // namespace rawrbox
