#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

namespace rawrbox {
	// PRIVATE ----
	std::vector<rawrbox::Decal> DECALS::_decals = {};

	std::unique_ptr<Diligent::DynamicBuffer> DECALS::_buffer;
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
		_decals.reserve(16); // OFFSET
		{
			// Init uniforms
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Decals::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::Vector4u);

			rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, nullptr, &uniforms);
			// -----------------------------------------
		}

		// Create data --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::Decal);
			BuffDesc.Name = "RawrBox::Decals::Buffer";
			BuffDesc.Usage = Diligent::USAGE_SPARSE;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * _decals.capacity();

			Diligent::DynamicBufferCreateInfo dynamicBuff;
			dynamicBuff.Desc = BuffDesc;

			_buffer = std::make_unique<Diligent::DynamicBuffer>(rawrbox::RENDERER->device(), dynamicBuff);
			_bufferRead = _buffer->GetBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// -----------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}, {_buffer->GetBuffer(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------

		// Update, DynamicBuffer does not support passing data?
		updateBuffer();
		// -----------
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

		rawrbox::Vector4u settings = {static_cast<uint32_t>(count()), 0, 0, 0};

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(uniforms, 0, sizeof(rawrbox::Vector4u), &settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------
	}

	void DECALS::updateBuffer() {
		if (_buffer == nullptr) throw _logger->error("Buffer not initialized! Did you call 'init' ?");
		if (!rawrbox::__DECALS_DIRTY__ || _decals.empty()) return;

		auto* device = rawrbox::RENDERER->device();
		auto* context = rawrbox::RENDERER->context();

		// Resize buffer ----
		uint64_t size = sizeof(rawrbox::Decal) * static_cast<uint64_t>(_decals.capacity());
		if (size > _buffer->GetDesc().Size) {
			_decals.reserve(_decals.capacity() + 16); // + OFFSET
			_buffer->Resize(device, context, sizeof(rawrbox::Decal) * static_cast<uint64_t>(_decals.capacity()), true);
		}
		// --------

		auto* buffer = _buffer->GetBuffer();

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{buffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(buffer, 0, sizeof(rawrbox::Decal) * static_cast<uint64_t>(_decals.size()), _decals.empty() ? nullptr : _decals.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{buffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ---------

		rawrbox::__DECALS_DIRTY__ = false;
	}

	void DECALS::update() {
		if (uniforms == nullptr) throw _logger->error("Buffer not initialized! Did you call 'init' ?");

		updateBuffer();    // Update all decals if dirty
		updateConstants(); // Update buffer if dirty
	}

	// UTILS ----
	Diligent::IBufferView* DECALS::getBuffer() { return _bufferRead; }
	const rawrbox::Decal& DECALS::get(size_t indx) {
		if (indx >= _decals.size()) throw _logger->error("Invalid decal index {}", indx);
		return _decals[indx];
	}

	size_t DECALS::count() { return _decals.size(); }
	// ----

	// DECALS ----
	void DECALS::add(const rawrbox::Decal& decal) {
		_decals.push_back(decal);

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

	void DECALS::clear() {
		if (_decals.empty()) return;
		_decals.clear();

		rawrbox::__DECALS_DIRTY__ = true;
		_CONSTANTS_DIRTY = true;
	}
	// ---------
} // namespace rawrbox
