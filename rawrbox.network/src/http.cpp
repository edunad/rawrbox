#include <rawrbox/network/http.hpp>

namespace rawrbox {
	void HTTP::request(const std::string& url, const rawrbox::HTTPMethod method, const std::map<std::string, std::string>& headers, std::function<void(int, std::map<std::string, std::string>, std::string)> callback, int timeout) {
		if (callback == nullptr) throw std::runtime_error("[RawrBox-HTTP] Invalid callback");

		cpr::Header header = {};

		cpr::UserAgent agent = "RawrBox/1.0 (Tabby) Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/89.0.4389.114 Safari/537.36";
		cpr::Timeout time = timeout;

		// Setup headers ---
		for (auto& h : headers) {
			header.emplace(h);
		}
		// ----

		// Setup callback ---
		auto cb = [callback](cpr::Response r) {
			std::map<std::string, std::string> headerResp = {};
			for (auto& h : r.header) {
				headerResp[h.first] = h.second;
			}

			if (r.error.code != cpr::ErrorCode::OK) {
				callback(r.status_code, headerResp, r.error.message);
			} else {
				callback(r.status_code, headerResp, r.text);
			}

			return r.text;
		};
		// ----

		switch (method) {
			case HTTPMethod::GET:
				cpr::GetCallback(cb,
				    cpr::Url{url}, header, agent, time);
				break;
			case HTTPMethod::POST:
				cpr::PostCallback(cb,
				    cpr::Url{url}, header, agent, time);
				break;
			case HTTPMethod::PUT:
				cpr::PutCallback(cb,
				    cpr::Url{url}, header, agent, time);
				break;
			case HTTPMethod::ERASE:
				cpr::DeleteCallback(cb,
				    cpr::Url{url}, header, agent, time);
				break;
			case HTTPMethod::OPTIONS:
				cpr::OptionsCallback(cb,
				    cpr::Url{url}, header, agent, time);
				break;
		};
	}
}; // namespace rawrbox
