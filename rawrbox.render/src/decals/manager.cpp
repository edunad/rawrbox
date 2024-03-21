#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::unique_ptr<rawrbox::Decal>> DECALS::_decals = {};

	std::unique_ptr<Diligent::DynamicBuffer> DECALS::_buffer = nullptr;
	Diligent::IBufferView* DECALS::_bufferRead = nullptr;
	bool DECALS::_CONSTANTS_DIRTY = false;

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> DECALS::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Decals");
	// -------------
	// -----------

	// PUBLIC ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::uniforms;
	// -------

	void DECALS::init() {
		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Decals::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::DecalsConstants);

			rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, nullptr, &uniforms);
		}
		// -----------------------------------------

		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::DecalVertex);
			BuffDesc.Name = "rawrbox::Decals::Buffer";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT; // Diligent::USAGE_SPARSE has some issues on the steam deck, but its only good for BIG buffers, in our case we use small buffers for light
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Size = BuffDesc.ElementByteStride * static_cast<uint64_t>(std::max<size_t>(_decals.size() + 32, 1));
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

			Diligent::DynamicBufferCreateInfo dynamicBuff;
			dynamicBuff.Desc = BuffDesc;

			_buffer = std::make_unique<Diligent::DynamicBuffer>(rawrbox::RENDERER->device(), dynamicBuff);
			_bufferRead = _buffer->GetBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}

		// BARRIER -----
		rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER},
		    {_buffer->GetBuffer(), Diligent::RESOURCE_STATE_SHADER_RESOURCE}});
		// -----------

		update();
	}

	void DECALS::shutdown() {
		RAWRBOX_DESTROY(uniforms);

		_bufferRead = nullptr;
		_buffer.reset();

		_decals.clear();
	}

	void DECALS::updateConstants() {
		if (!_CONSTANTS_DIRTY) return;
		_CONSTANTS_DIRTY = false;

		rawrbox::DecalsConstants settings = {static_cast<uint32_t>(count())};

		// BARRIER ----
		rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{uniforms, Diligent::RESOURCE_STATE_COPY_DEST}});
		rawrbox::RENDERER->context()->UpdateBuffer(uniforms, 0, sizeof(rawrbox::DecalsConstants), &settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER}});
		// --------
	}

	void DECALS::update() {
		if (_buffer == nullptr) throw _logger->error("Buffer not initialized! Did you call 'init' ?");
		if (!rawrbox::__DECALS_DIRTY__ || _decals.empty()) return;

		// Update decals ---
		std::vector<rawrbox::DecalVertex> decals = {};
		decals.reserve(_decals.size());

		for (auto& d : _decals) {
			rawrbox::DecalVertex decal = {};

			decal.data = {d->textureID, d->textureAtlasIndex, 0, 0};
			decal.worldToLocal = d->localToWorld.inverse().transpose();
			decal.color = d->color;

			decals.push_back(decal);
		}

		auto* context = rawrbox::RENDERER->context();
		auto* device = rawrbox::RENDERER->device();

		// Update buffer ----
		uint64_t size = sizeof(rawrbox::DecalVertex) * static_cast<uint64_t>(_decals.size());
		if (size > _buffer->GetDesc().Size) _buffer->Resize(device, context, size + 32); // + OFFSET

		auto* buffer = _buffer->GetBuffer();

		// BARRIER ----
		rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{buffer, Diligent::RESOURCE_STATE_COPY_DEST}});
		context->UpdateBuffer(buffer, 0, sizeof(rawrbox::DecalVertex) * static_cast<uint64_t>(_decals.size()), decals.empty() ? nullptr : decals.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{buffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE}});
		// ---------

		rawrbox::__DECALS_DIRTY__ = false;
		// -------
	}

	void DECALS::bindUniforms() {
		if (uniforms == nullptr) throw _logger->error("Buffer not initialized! Did you call 'init' ?");

		updateConstants();
		update(); // Update all decals if dirty
	}

	// UTILS ----
	Diligent::IBufferView* DECALS::getBuffer() { return _bufferRead; }
	rawrbox::Decal* DECALS::get(size_t indx) {
		if (indx >= _decals.size()) return nullptr;
		return _decals[indx].get();
	}

	size_t DECALS::count() { return _decals.size(); }
	// ----

	// DECALS ----
	void DECALS::add(const rawrbox::Decal& decal) {
		auto ptr = std::make_unique<rawrbox::Decal>(decal);
		_decals.push_back(std::move(ptr));

		rawrbox::__DECALS_DIRTY__ = true;
		_CONSTANTS_DIRTY = true;
	}

	bool DECALS::remove(size_t indx) {
		if (indx > _decals.size()) return false;

		_decals.erase(_decals.begin() + indx);
		rawrbox::__DECALS_DIRTY__ = true;
		_CONSTANTS_DIRTY = true;
		return true;
	}

	bool DECALS::remove(const rawrbox::Decal& decal) {
		if (_decals.empty()) return false;

		for (size_t i = 0; i < _decals.size(); i++) {
			if (_decals[i].get() == &decal) {
				_decals.erase(_decals.begin() + i);

				rawrbox::__DECALS_DIRTY__ = true;
				_CONSTANTS_DIRTY = true;
				return true;
			}
		}

		return false;
	}

	void DECALS::clear() {
		if (_decals.empty()) return;
		_decals.clear();

		rawrbox::__DECALS_DIRTY__ = true;
		_CONSTANTS_DIRTY = true;
	}
	// ---------
} // namespace rawrbox
