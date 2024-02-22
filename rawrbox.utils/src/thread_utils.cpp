#include <rawrbox/utils/thread_utils.hpp>

#ifdef _MSC_VER
	#include <windows.h>
#endif

namespace rawrbox {
#ifdef _WIN32
	#pragma pack(push, 8)
	using THREADNAME_INFO = struct tagTHREADNAME_INFO {
		DWORD dwType;     // Must be 0x1000.
		LPCSTR szName;    // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags;    // Reserved for future use, must be zero.
	};
	#pragma pack(pop)

	void ThreadUtils::setName(const std::string& name) {
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = name.c_str();
		info.dwThreadID = GetCurrentThreadId();
		info.dwFlags = 0;
	}
#else
	void ThreadUtils::setName(const std::string& name) {}
#endif
} // namespace rawrbox
