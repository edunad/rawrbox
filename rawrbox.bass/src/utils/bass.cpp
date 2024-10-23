
#include <rawrbox/bass/utils/bass.hpp>
#include <rawrbox/utils/logger.hpp>

#include <bass.h>
#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ---
	std::unique_ptr<rawrbox::Logger> BASSUtils::_logger = std::make_unique<rawrbox::Logger>("RawrBox-BASS");
	// ----------

	void BASSUtils::checkBASSError() {
		int err = BASS_ErrorGetCode();
		std::string readErr;

		switch (err) {
			case BASS_OK:
				return;
			case BASS_ERROR_INIT:
				readErr = "BASS has not been initialized";
				break;
			case BASS_ERROR_START:
				readErr = "BASS has not been started";
				break;
			case BASS_ERROR_NO3D:
				readErr = "Audio not in 3D format (mono channel)";
				break;
			case BASS_ERROR_NOTAUDIO:
			case BASS_ERROR_FORMAT:
			case BASS_ERROR_FILEFORM:
				readErr = "Invalid audio format";
				break;
			case BASS_ERROR_HANDLE:
				readErr = "Invalid audio handle. Is the channel still alive?";
				break;
			case BASS_ERROR_ILLTYPE:
				readErr = "Invalid audio type";
				break;
			case BASS_ERROR_NOTAVAIL:
				readErr = "Requested data not available";
				break;
			default:
				readErr = fmt::format("Unknown error {}, check http://www.un4seen.com/doc/#bass/BASS_ErrorGetCode.html", err);
				break;
		}

		RAWRBOX_CRITICAL("Bass audio error:\n\t{}", readErr);
	}

} // namespace rawrbox
