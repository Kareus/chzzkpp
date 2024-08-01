#include <chzzkpp/ChzzkCore.h>
#include <chzzkpp/Path.h>
#include <chzzkpp/ChzzkUtils.h>

#ifdef _WIN32

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "wldap32.lib")
#pragma comment (lib, "crypt32.lib")

#endif

namespace chzzkpp
{

#if _USE_CURL
	size_t curl_write_string_callback(void* contents, size_t size, size_t nmemb, std::string* s)
	{
		size_t newLength = size * nmemb;

		try
		{
			s->append((char*)contents, newLength);
		}
		catch (std::bad_alloc&)
		{
			return 0;
		}

		return newLength;
	}

	ChzzkCore::ChzzkCore(int timeout) : timeout(timeout)
	{
		_hasAuth = false;
		authKeys = { "", "" };

		curl = curl_easy_init();

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_string_callback);
	}

	ChzzkCore::~ChzzkCore()
	{
		curl_easy_cleanup(curl);
		curl = nullptr;
	}

	std::string ChzzkCore::request(const std::string& path)
	{
		if (!curl)
		{
			throw std::exception("Core is not initialized.");
			return "";
		}

		CURLcode result;
		std::string response;

		curl_easy_setopt(curl, CURLOPT_URL, path.c_str());

		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		curl_slist* slist = nullptr;

		if (_hasAuth)
		{
			std::string auth = "Cookie: NID_AUT=" + authKeys.first + ";NID_SES=" + authKeys.second;

			slist = curl_slist_append(slist, auth.c_str());
		}

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);

		result = curl_easy_perform(curl);

		if (result != CURLE_OK)
			response = curl_easy_strerror(result);

		curl_slist_free_all(slist);

		return response;
	}

#endif

	void ChzzkCore::setAuth(const std::string& auth, const std::string& session)
	{
		authKeys.first = auth;
		authKeys.second = session;

		_hasAuth = true;
	}

	void ChzzkCore::clearAuth()
	{
		authKeys.first = "";
		authKeys.second = "";
		_hasAuth = false;
	}

	void ChzzkCore::setTimeout(int timeout)
	{
		this->timeout = timeout;
	}

	int ChzzkCore::getTimeout() const
	{
		return timeout;
	}

	std::string ChzzkCore::getChannel(const std::string& channelID)
	{
		return request(getChannelPath(channelID));
	}

	std::string ChzzkCore::getLiveStatus(const std::string& channelID)
	{
		return request(getLiveStatusPath(channelID));
	}

	std::string ChzzkCore::getLiveDetail(const std::string& channelID)
	{
		return request(getLiveDetailPath(channelID));
	}

	std::string ChzzkCore::getVideo(int videoNo)
	{
		return request(getVideoPath(videoNo));
	}

	std::string ChzzkCore::getTopViewerLives(int size)
	{
		return request(getTopViewerLivesPath(size));
	}

	std::string ChzzkCore::getTopViewerLives(const std::string& keyword, int size)
	{
		return request(getTopViewerLivesPath(keyword, size));
	}

	std::string ChzzkCore::getRecommendationChannels(bool partner)
	{
		return request(partner ? CHZZK_API_PATH_RECOMMENDATION_PARTNERS : CHZZK_API_PATH_RECOMMENDATION_CHANNELS);
	}

	std::string ChzzkCore::getRecommendationLives()
	{
		return request(CHZZK_API_PATH_RECOMMENDATION_LIVES);
	}

	std::string ChzzkCore::getMissions(const std::string& channelID, bool mine, int page, int size)
	{
		return request(getChannelMissionsPath(channelID, mine, page, size));
	}

	std::string ChzzkCore::getChatDonationSetting(const std::string& channelID)
	{
		return request(getChatDonationSettingPath(channelID));
	}

	std::string ChzzkCore::getVideoDonationSetting(const std::string& channelID)
	{
		return request(getVideoDonationSettingPath(channelID));
	}

	std::string ChzzkCore::getMissionDonationSetting(const std::string& channelID)
	{
		return request(getMissionDonationSettingPath(channelID));
	}

	std::string ChzzkCore::getUserData()
	{
		return request(CHZZK_API_PATH_USER);
	}

	std::string ChzzkCore::getAccessToken(const std::string& chatChannelID)
	{
		return request(getAccessTokenPath(chatChannelID));
	}

	std::string ChzzkCore::searchChannel(const std::string& keyword, int offset, int size, bool withFirstChannelContent)
	{
		return request(getSearchChannelPath(encodeURL(keyword), offset, size, withFirstChannelContent));
	}

	std::string ChzzkCore::searchLive(const std::string& keyword, int offset, int size)
	{
		return request(getSearchLivePath(encodeURL(keyword), offset, size));
	}

	std::string ChzzkCore::searchVideo(const std::string& keyword, int offset, int size)
	{
		return request(getSearchVideoPath(encodeURL(keyword), offset, size));
	}

	bool ChzzkCore::hasAuth() const
	{
		return _hasAuth;
	}
}