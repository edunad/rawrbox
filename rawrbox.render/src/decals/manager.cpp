#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// PRIVATE ----
	std::vector<std::unique_ptr<rawrbox::Decal>> DECALS::_decals = {};

	std::unique_ptr<Diligent::DynamicBuffer> DECALS::_buffer = nullptr;
	Diligent::IBufferView* DECALS::_bufferRead = nullptr;
	// -----------

	// PUBLIC ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> DECALS::uniforms;
	// -------

	void DECALS::init() {
		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Decals::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffDesc.Size = sizeof(rawrbox::DecalConstants);

			rawrbox::RENDERER->device()->CreateBuffer(BuffDesc, nullptr, &uniforms);
		}
		// -----------------------------------------

		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::DecalVertex);
			BuffDesc.Name = "rawrbox::Decals::Buffer";
			BuffDesc.Usage = Diligent::USAGE_SPARSE;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Size = BuffDesc.ElementByteStride * static_cast<uint64_t>(std::max<size_t>(_decals.size(), 1));
			BuffDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

			Diligent::DynamicBufferCreateInfo dynamicBuff;
			dynamicBuff.Desc = BuffDesc;

			_buffer = std::make_unique<Diligent::DynamicBuffer>(rawrbox::RENDERER->device(), dynamicBuff);
			_bufferRead = _buffer->GetBuffer()->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);
		}

		update();
	}

	void DECALS::shutdown() {
		RAWRBOX_DESTROY(uniforms);

		_bufferRead = nullptr;
		_buffer.reset();

		_decals.clear();
	}

	void DECALS::update() {
		if (_buffer == nullptr) throw std::runtime_error("[RawrBox-DECALS] Buffer not initialized! Did you call 'init' ?");
		if (!rawrbox::__DECALS_DIRTY__ || _decals.empty()) return;

		auto context = rawrbox::RENDERER->context();
		auto device = rawrbox::RENDERER->device();

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

		// Update buffer ----
		uint64_t size = sizeof(rawrbox::DecalVertex) * static_cast<uint64_t>(std::max<size_t>(_decals.size(), 1)); // Always keep 1
		if (size > _buffer->GetDesc().Size) {
			_buffer->Resize(device, context, size, true);
		}

		rawrbox::RENDERER->context()->UpdateBuffer(_buffer->GetBuffer(), 0, sizeof(rawrbox::DecalVertex) * static_cast<uint64_t>(_decals.size()), decals.empty() ? nullptr : decals.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		rawrbox::__DECALS_DIRTY__ = false;
		// -------
	}

	void DECALS::bindUniforms() {
		if (uniforms == nullptr) throw std::runtime_error("[RawrBox-DECALS] Buffer not initialized! Did you call 'init' ?");
		update(); // Update all lights if dirty

		{
			Diligent::MapHelper<rawrbox::DecalConstants> CBConstants(rawrbox::RENDERER->context(), uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			CBConstants->settings = {static_cast<uint32_t>(rawrbox::DECALS::count()), 0, 0, 0};
		}
		// --------------
	}

	// UTILS ----
	Diligent::IBufferView* DECALS::getBuffer() { return _bufferRead; }
	rawrbox::Decal* DECALS::getDecal(size_t indx) {
		if (indx >= _decals.size()) return nullptr;
		return _decals[indx].get();
	}

	size_t DECALS::count() { return _decals.size(); }
	// ----

	// DECALS ----
	void DECALS::add(rawrbox::Decal decal) {
		auto ptr = std::make_unique<rawrbox::Decal>(decal);
		_decals.push_back(std::move(ptr));

		rawrbox::__DECALS_DIRTY__ = true;
	}

	bool DECALS::remove(size_t indx) {
		if (indx > _decals.size()) return false;

		_decals.erase(_decals.begin() + indx);
		rawrbox::__DECALS_DIRTY__ = true;
		return true;
	}

	bool DECALS::remove(rawrbox::Decal* decal) {
		if (decal == nullptr || _decals.empty()) return false;

		for (size_t i = 0; i < _decals.size(); i++) {
			if (_decals[i].get() == decal) {
				_decals.erase(_decals.begin() + i);
				rawrbox::__DECALS_DIRTY__ = true;
				break;
			}
		}

		return true;
	}
	// ---------
} // namespace rawrbox
