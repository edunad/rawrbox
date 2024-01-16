#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	PostProcessPlugin::~PostProcessPlugin() {
		this->_postProcesses.clear();
		RAWRBOX_DESTROY(this->_buffer);
	}

	void PostProcessPlugin::initialize(const rawrbox::Vector2i& /*size*/) {
		Diligent::BufferDesc BuffPixelDesc;
		BuffPixelDesc.Name = "rawrbox::PostProcess";
		BuffPixelDesc.Usage = Diligent::USAGE_DYNAMIC;
		BuffPixelDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		BuffPixelDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		BuffPixelDesc.Size = sizeof(rawrbox::BindlessPostProcessBuffer);

		rawrbox::RENDERER->device()->CreateBuffer(BuffPixelDesc, nullptr, &this->_buffer);
		rawrbox::BindlessManager::barrier(*this->_buffer, rawrbox::BufferType::CONSTANT);
	}

	void PostProcessPlugin::upload() {
		for (const auto& _postProcess : this->_postProcesses) {
			_postProcess->init();
		}
	}

	void PostProcessPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& sig, bool compute) {
		if (compute) return;
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "PostProcessConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
	}

	void PostProcessPlugin::bind(Diligent::IPipelineResourceSignature& sig, bool compute) {
		if (compute) return;
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "PostProcessConstants")->Set(this->_buffer);
	}

	void PostProcessPlugin::postRender(rawrbox::TextureRender& renderTexture) {
		for (const auto& process : this->_postProcesses) {
			renderTexture.startRecord(false);
			process->applyEffect(renderTexture);
			renderTexture.stopRecord();
		}
	}

	const std::string PostProcessPlugin::getID() const { return "PostProcess"; }

	// Post utils ----
	void PostProcessPlugin::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) throw this->_logger->error("Failed to remove {}!", indx);
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
	}

	rawrbox::PostProcessBase& PostProcessPlugin::get(size_t indx) const {
		if (indx >= this->_postProcesses.size()) throw this->_logger->error("Failed to get {}!", indx);
		return *this->_postProcesses[indx];
	}

	Diligent::IBuffer* PostProcessPlugin::getBuffer() const { return this->_buffer; }
	size_t PostProcessPlugin::count() { return this->_postProcesses.size(); }
	// ----
} // namespace rawrbox
