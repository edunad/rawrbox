#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

#include <string>

namespace rawrbox {
	class TextureImage : public TextureBase {
	private:
		std::vector<unsigned char> _pixels;
		bool _failedToLoad = false;
		std::string _name = "IMAGE-TEXTURE";

	public:
		explicit TextureImage(const std::string& fileName, bool useFallback = true);

		// ------ PIXEL-UTILS
		virtual void setName(const std::string& name);
		virtual void resize(const rawrbox::Vector2i& newsize);
		// --------------------

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count) override;
	};
} // namespace rawrbox
