#pragma once

#ifndef _CHZZK_CLIENT_
#define _CHZZK_CLIENT_

#include "ChzzkTypes.h"
#include "ChzzkCore.h"

#include <nlohmann/json.hpp>

namespace chzzkpp
{
	//parse and process api raw json data from core
	class ChzzkClient
	{
		ChzzkCore* core;

		nlohmann::json getContent(const std::string& data);

	public:
		ChzzkClient(ChzzkCore* core);

		ChzzkChannel getChannel(const std::string& channelID);

		ChzzkLiveStatus getLiveStatus(const std::string& channelID);

		ChzzkLiveDetail getLiveDetail(const std::string& channelID);

		ChzzkVideo getVideo(int videoNo);

		//gets live list in viewers count order
		ChzzkRecommendResult getRecommendationLives(int size = 30);

		//gets live list with keyword in viewers count order
		//keyword should be utf8
		ChzzkRecommendResult getRecommendationLives(const std::string& keyword, int size = 30);

		//you should be logged in (with ChzzkCore::setAuth) to get user data
		ChzzkUserData getUserData();

		ChzzkAccessToken getAccessToken(const std::string& chatChannelID);

		//keyword should be utf8
		ChzzkChannelResult searchChannel(const std::string& keyword, int offset = 0, int size = 20, bool withFirstChannelContent = false);

		//keyword should be utf8
		ChzzkLiveResult searchLive(const std::string& keyword, int offset = 0, int size = 20);

		//keyword should be utf8
		ChzzkVideoResult searchVideo(const std::string& keyword, int offset = 0, int size = 20);

		ChzzkCore* getCore();
	};
}
#endif