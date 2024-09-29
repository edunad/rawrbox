#pragma once

#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceGLTF : public rawrbox::Resource {
		std::unique_ptr<rawrbox::GLTFImporter> _model = nullptr;

	public:
		ResourceGLTF() = default;
		ResourceGLTF(const ResourceGLTF&) = delete;
		ResourceGLTF(ResourceGLTF&&) = delete;
		ResourceGLTF& operator=(const ResourceGLTF&) = delete;
		ResourceGLTF& operator=(ResourceGLTF&&) = delete;
		~ResourceGLTF() override;

		bool load(const std::vector<uint8_t>& buffer) override;
		[[nodiscard]] rawrbox::GLTFImporter* get() const;
	};

	class GLTFLoader : public rawrbox::Loader {
	public:
		GLTFLoader() = default;
		GLTFLoader(const GLTFLoader&) = delete;
		GLTFLoader(GLTFLoader&&) = delete;
		GLTFLoader& operator=(const GLTFLoader&) = delete;
		GLTFLoader& operator=(GLTFLoader&&) = delete;
		~GLTFLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry() override;

		bool canLoad(const std::string& fileExtention) override;
		bool supportsBuffer(const std::string& fileExtention) override;
	};
} // namespace rawrbox
