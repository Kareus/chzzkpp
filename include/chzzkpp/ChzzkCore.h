#pragma once

#ifndef _CHZZK_CORE_
#define _CHZZK_CORE_

#include "Config.h"

#include <string>

#if _USE_CURL
#include <curl/curl.h>
#endif

namespace chzzkpp
{

#if _USE_CURL
	//reference: https://stackoverflow.com/questions/2329571/c-libcurl-get-output-into-a-string
	size_t curl_write_string_callback(void* contents, size_t size, size_t nmemb, std::string* s);
#endif
	
	//core on Chzzk APi using libcurl
	//initializes curl and gets raw string data from api
	class ChzzkCore
	{
#if _USE_CURL
		CURL* curl;
#endif

		bool _hasAuth;
		int timeout;

		std::pair<std::string, std::string> authKeys;

	public:

		//@timeout respond timeout seconds. never times out if value is 0
		ChzzkCore(int timeout = 0);
		~ChzzkCore();

		std::string request(const std::string& path);

		//you can set NID_AUTH and NID_SESSION here.
		//you can get these from NID_AUT and NID_SES cookies from Application > Cookies > https://chzzk.naver.com, after you logged in
		void setAuth(const std::string& auth, const std::string& session);

		void clearAuth();

		//@timeout respond timeout seconds. never times out if value is 0
		void setTimeout(int timeout);

		int getTimeout() const;

		std::string getChannel(const std::string& channelID);

		std::string getLiveStatus(const std::string& channelID);

		std::string getLiveDetail(const std::string& channelID);

		std::string getVideo(int videoNo);

		std::string getTopViewerLives(int size = 30);

		std::string getUserData();

		std::string getAccessToken(const std::string& chatChannelID);

		//keyword should be utf8
		std::string getTopViewerLives(const std::string& keyword, int size = 30);

		//get recommendation channels
		std::string getRecommendationChannels(bool partner = false);

		//get recommendation lives
		std::string getRecommendationLives();

		//keyword should be utf8
		std::string searchChannel(const std::string& keyword, int offset = 0, int size = 20, bool withFirstChannelContent = false);

		//keyword should be utf8
		std::string searchLive(const std::string& keyword, int offset = 0, int size = 20);

		//keyword should be utf8
		std::string searchVideo(const std::string& keyword, int offset = 0, int size = 20);

		//you should be logged in to view your mission donations, if mine is true
		std::string getMissions(const std::string& channelID, bool mine = false, int page = 0, int size = 50);

		std::string getChatDonationSetting(const std::string& channelID);

		std::string getVideoDonationSetting(const std::string& channelID);

		std::string getMissionDonationSetting(const std::string& channelID);

		bool hasAuth() const;
	};
}

#endif