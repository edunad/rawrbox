#pragma once

#include <rawrbox/render/plugins/base.hpp>
#include <rawrbox/render/post_process/base.hpp>

namespace rawrbox {
	class PostProcessPlugin : public rawrbox::RenderPlugin {
	protected:
		// BUFFERS ---
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _buffer;
		// -------

		std::vector<std::unique_ptr<rawrbox::PostProcessBase>> _postProcesses = {};

	public:
		PostProcessPlugin() = default;
		PostProcessPlugin(const PostProcessPlugin&) = delete;
		PostProcessPlugin(PostProcessPlugin&&) = delete;
		PostProcessPlugin& operator=(const PostProcessPlugin&) = delete;
		PostProcessPlugin& operator=(PostProcessPlugin&&) = delete;
		~PostProcessPlugin() override;

		void initialize(const rawrbox::Vector2u& size) override;
		void upload() override;

		void signatures(std::vector<Diligent::PipelineResourceDesc>& sig, bool compute) override;
		void bindStatic(Diligent::IPipelineResourceSignature& sig, bool compute) override;
		void postRender(rawrbox::TextureRender& render) override;

		// Process utils ----
		template <class T = rawrbox::PostProcessBase, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::PostProcessBase>)
		void add(CallbackArgs&&... args) {
			this->_postProcesses.push_back(std::make_unique<T>(std::forward<CallbackArgs>(args)...));
		}

		virtual void remove(size_t indx);
		[[nodiscard]] virtual rawrbox::PostProcessBase& get(size_t indx) const;
		[[nodiscard]] virtual Diligent::IBuffer* getBuffer() const;
		virtual size_t count();

		std::string getID() override;
		// ---------
	};
} // namespace rawrbox
