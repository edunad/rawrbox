#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/stencil.h>

#include <bgfx/bgfx.h>

namespace rawrBox {
	class Renderer {
		private:
			// Default settings
			uint32_t _clearColor = 0x000000FF;
			bgfx::ViewId _id = 0;
			// -----

			std::shared_ptr<rawrBox::Stencil> _stencil;
		public:
			~Renderer();
			Renderer(bgfx::ViewId id, const rawrBox::Vector2i& size);

			void initialize();
			void setClearColor(uint32_t clearColor);
			void resizeView(const rawrBox::Vector2i& pos, const rawrBox::Vector2i& size);
			void clear();

			#pragma region RENDERING
			void swapBuffer() const;
			void render() const;
			#pragma endregion

			#pragma region UTILS
			bgfx::ViewId getID() const;
			std::shared_ptr<rawrBox::Stencil> getStencil() const;
			#pragma endregion
	};
}
