#pragma once
#ifndef _CHZZK_UTILS_
#define _CHZZK_UTILS_

#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include "ChzzkTypes.h"

namespace chzzkpp
{
	std::string convertUTF8(const std::string& str);

	std::string encodeURL(const std::string& str);

	//sets dest with json[key] if exists and not null, otherwise with default value
	template <typename T>
	void json_safe_get(T& dest, const nlohmann::json& json, const std::string& key)
	{
		if (json.find(key) != json.end() && !json[key].is_null()) dest = json[key];
		else dest = T();
	}

	template <typename T>
	T json_safe_get(const nlohmann::json& json)
	{
		T ret;

		try
		{
			ret = json;
		}
		catch (const nlohmann::json::exception& e)
		{
			ret = T();
		}

		return ret;
	}

	template <typename T>
	T parse(nlohmann::json json);

	template <typename T>
	T parse_raw(std::string raw_data)
	{
		nlohmann::json json = nlohmann::json::parse(raw_data);
		return parse<T>(json);
	}

	std::string getLiveStatusPath(const std::string& channelID);

	std::string getLiveDetailPath(const std::string& channelID);

	std::string getChannelPath(const std::string& channelID);

	std::string getVideoPath(int videoNo);

	std::string getTopViewerLivesPath(int size);

	std::string getTopViewerLivesPath(const std::string& keyword, int size);

	std::string getSearchChannelPath(const std::string& keyword, int offset, int size, bool withFirstChannelContent);

	std::string getSearchLivePath(const std::string& keyword, int offset, int size);

	std::string getSearchVideoPath(const std::string& keyword, int offset, int size);

	std::string getAccessTokenPath(const std::string& chatChannelID, const std::string& chatType = "STREAMING");

	std::string getChannelMissionsPath(const std::string& channelID, bool mine, int page, int size);

	std::string getChatDonationSettingPath(const std::string& channelID);

	std::string getVideoDonationSettingPath(const std::string& channelID);

	std::string getMissionDonationSettingPath(const std::string& channelID);
}
#endif