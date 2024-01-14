#pragma once

#include <rawrbox/render/plugins/base.hpp>
#include <rawrbox/render/post_process/base.hpp>

namespace rawrbox {
	class PostProcessPlugin : public rawrbox::RenderPlugin {
	protected:
		std::vector<std::unique_ptr<rawrbox::PostProcessBase>> _postProcesses = {};
		std::unique_ptr<rawrbox::TextureRender> _rt = {};

	public:
		PostProcessPlugin() = default;
		PostProcessPlugin(const PostProcessPlugin&) = delete;
		PostProcessPlugin(PostProcessPlugin&&) = delete;
		PostProcessPlugin& operator=(const PostProcessPlugin&) = delete;
		PostProcessPlugin& operator=(PostProcessPlugin&&) = delete;
		~PostProcessPlugin() override;

		// Process utils ----
		template <class T, typename... CallbackArgs>
		void add(CallbackArgs&&... args) {
			this->_postProcesses.push_back(std::make_unique<T>(std::forward<CallbackArgs>(args)...));
		}

		virtual void remove(size_t indx);
		[[nodiscard]] virtual rawrbox::PostProcessBase& get(size_t indx) const;
		virtual size_t count();
		// ---------

		[[nodiscard]] const std::string getID() const override;

		void initialize(const rawrbox::Vector2i& size) override;
		void resize(const rawrbox::Vector2i& size) override;

		void postRender(const rawrbox::TextureRender& render) override;
	};
} // namespace rawrbox
