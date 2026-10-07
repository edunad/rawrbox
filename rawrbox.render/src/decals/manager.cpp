#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

#include <algorithm>

namespace rawrbox {
	// PRIVATE ----
	std::vector<rawrbox::Decal> DECALS::_decals = {};

	// BINNING ---
	std::vector<rawrbox::Decal> DECALS::_sorted = {};

	std::vector<rawrbox::Vector4f> DECALS::_viewBounds = {};
	std::vector<rawrbox::Vector4f> DECALS::_bounds = {};

	std::vector<rawrbox::ZBinEntry> DECALS::_entries = {};
	std::vector<uint32_t> DECALS::_zbins = {};

	rawrbox::Matrix4x4 DECALS::_zbinView = {};
	rawrbox::Vector2f DECALS::_zbinNearFar = {};
	// -----------

	// BUFFERS ---
	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::_buffer;
	Diligent::IBufferView* DECALS::_bufferRead = nullptr;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::_zbinBuffer;
	Diligent::IBufferView* DECALS::_zbinBufferRead = nullptr;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::_boundsBuffer;
	Diligent::IBufferView* DECALS::_boundsBufferRead = nullptr;
	// -----------

	bool DECALS::_CONSTANTS_DIRTY = false;
	bool DECALS::_BINNED = true;

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> DECALS::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Decals");
	// -------------

	// PUBLIC ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::uniforms;
	// -------

	void DECALS::init() {
		_decals.reserve(16); // OFFSET

		auto* device = rawrbox::RENDERER->device();

		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Decals::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.Size = sizeof(rawrbox::Vector4u);

			device->CreateBuffer(BuffDesc, nullptr, &uniforms);
		}
		// -----------------------------------------

		// Create data --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::Decal);
			BuffDesc.Name = "RawrBox::Decals::Buffer";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * RB_RENDER_MAX_DECALS;

			device->CreateBuffer(BuffDesc, nullptr, &_buffer);
			_bufferRead = _buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// -----------

		// Zbins --
		{
			std::vector<uint32_t> empty(RB_RENDER_ZBINS, rawrbox::ClusteredUtils::EMPTY_BIN);

			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(uint32_t);
			BuffDesc.Name = "RawrBox::Decals::ZBins";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * RB_RENDER_ZBINS;

			Diligent::BufferData data;
			data.pData = empty.data();
			data.DataSize = BuffDesc.Size;

			device->CreateBuffer(BuffDesc, &data, &_zbinBuffer);
			_zbinBufferRead = _zbinBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// --------------

		// Create bounds --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::Vector4f);
			BuffDesc.Name = "RawrBox::Decals::Bounds";
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			BuffDesc.Size = BuffDesc.ElementByteStride * RB_RENDER_MAX_DECALS;

			device->CreateBuffer(BuffDesc, nullptr, &_boundsBuffer);
			_boundsBufferRead = _boundsBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}
		// --------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {_zbinBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {_boundsBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------

		_CONSTANTS_DIRTY = true;
	}

	void DECALS::shutdown() {
		RAWRBOX_DESTROY(uniforms);

		_bufferRead = nullptr;
		RAWRBOX_DESTROY(_buffer);

		_zbinBufferRead = nullptr;
		RAWRBOX_DESTROY(_zbinBuffer);

		_boundsBufferRead = nullptr;
		RAWRBOX_DESTROY(_boundsBuffer);

		_decals.clear();
		_sorted.clear();
		_viewBounds.clear();
		_bounds.clear();
		_entries.clear();
		_zbins.clear();
	}

	void DECALS::updateConstants() {
		if (!_CONSTANTS_DIRTY) return;
		_CONSTANTS_DIRTY = false;

		const auto total = _BINNED ? static_cast<uint32_t>(std::min<size_t>(_decals.size(), RB_RENDER_MAX_DECALS)) : 0U;
		rawrbox::Vector4u settings = {total, 0, 0, 0};

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(uniforms, 0, sizeof(rawrbox::Vector4u), &settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// --------
	}

	bool DECALS::updateBins(const rawrbox::CameraBase& camera) {
		const auto& view = camera.getViewMtx();
		const rawrbox::Vector2f nearFar = {rawrbox::MAIN_CAMERA->getZNear(), rawrbox::MAIN_CAMERA->getZFar()}; // Must match SCamera NearFar (shared, main camera)

		if (!rawrbox::__DECALS_DIRTY__ && _zbinView == view && _zbinNearFar == nearFar) return false;
		rawrbox::__DECALS_DIRTY__ = false;

		_zbinView = view;
		_zbinNearFar = nearFar;

		// Sort by depth + bin ---
		_entries.clear();
		_viewBounds.resize(std::min<size_t>(_decals.size(), RB_RENDER_MAX_DECALS));

		for (size_t i = 0; i < _decals.size(); i++) {
			if (i >= RB_RENDER_MAX_DECALS) {
				_logger->warn("Too many decals, only the first {} will be rendered", RB_RENDER_MAX_DECALS);
				break;
			}

			_viewBounds[i] = rawrbox::ClusteredUtils::toViewSphere(view, _decals[i].bounds);
			_entries.push_back(rawrbox::ClusteredUtils::getEntry(_viewBounds[i], static_cast<uint32_t>(i)));
		}

		rawrbox::ClusteredUtils::build(_entries, 0, nearFar.x, nearFar.y, _zbins);

		_sorted.clear();
		_bounds.clear();

		for (const auto& entry : _entries) {
			_sorted.push_back(_decals[entry.index]);
			_bounds.push_back(_viewBounds[entry.index]);
		}
		// ----

		// BARRIER ----
		auto* context = rawrbox::RENDERER->context();

		if (!_sorted.empty()) {
			rawrbox::BarrierUtils::barrier({{_buffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->UpdateBuffer(_buffer, 0, sizeof(rawrbox::Decal) * static_cast<uint64_t>(_sorted.size()), _sorted.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			rawrbox::BarrierUtils::barrier({{_buffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});

			rawrbox::BarrierUtils::barrier({{_boundsBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->UpdateBuffer(_boundsBuffer, 0, sizeof(rawrbox::Vector4f) * static_cast<uint64_t>(_bounds.size()), _bounds.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			rawrbox::BarrierUtils::barrier({{_boundsBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		}

		rawrbox::BarrierUtils::barrier({{_zbinBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateBuffer(_zbinBuffer, 0, sizeof(uint32_t) * static_cast<uint64_t>(_zbins.size()), _zbins.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{_zbinBuffer, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ---------

		return true;
	}

	bool DECALS::update(const rawrbox::CameraBase& camera, bool binned) {
		if (uniforms == nullptr) RAWRBOX_CRITICAL("Buffer not initialized! Did you call 'init' ?");

		if (_BINNED != binned) {
			_BINNED = binned;
			_CONSTANTS_DIRTY = true;
		}

		bool rebuilt = false;
		if (binned) rebuilt = updateBins(camera); // Sort + bin

		updateConstants(); // Update constants if dirty
		return rebuilt;
	}

	// UTILS ----
	Diligent::IBufferView* DECALS::getBuffer() { return _bufferRead; }
	Diligent::IBufferView* DECALS::getZBinBuffer() { return _zbinBufferRead; }
	Diligent::IBufferView* DECALS::getBoundsBuffer() { return _boundsBufferRead; }

	const rawrbox::Decal& DECALS::get(size_t indx) {
		if (indx >= _decals.size()) RAWRBOX_CRITICAL("Invalid decal index {}", indx);
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
		if (indx >= _decals.size()) return false;

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
