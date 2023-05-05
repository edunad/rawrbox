#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

#include <string>

namespace rawrBox {
	class TextureImage : public TextureBase {
	private:
		std::vector<unsigned char> _pixels;
		bool _failedToLoad = false;
		std::string _name = "IMAGE-TEXTURE";

	public:
		explicit TextureImage(const std::string& fileName, bool useFallback = true);

		// ------ PIXEL-UTILS
		virtual rawrBox::Colori getPixel(unsigned int x, unsigned int y);
		virtual rawrBox::Colori getPixel(const rawrBox::Vector2i& pos);

		virtual void setPixel(unsigned int x, unsigned int y, const rawrBox::Colori& col);
		virtual void setPixel(const rawrBox::Vector2i& pos, const rawrBox::Colori& col);

		virtual void setName(const std::string& name);
		virtual void resize(const rawrBox::Vector2i& newsize);
		// --------------------

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count) override;
	};
} // namespace rawrBox
