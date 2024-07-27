#include <chzzkpp/ChzzkUtils.h>
#include <chzzkpp/Path.h>

#if _WIN32
#include <Windows.h>
#endif

namespace chzzkpp
{
	std::string convertUTF8(const std::string& str)
	{
#if _WIN32
		int size = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, str.c_str(),
			str.length(), nullptr, 0);

		std::wstring utf16_str(size, '\0');
		MultiByteToWideChar(CP_ACP, MB_COMPOSITE, str.c_str(),
			str.length(), &utf16_str[0], size);

		int utf8_size = WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
			utf16_str.length(), nullptr, 0,
			nullptr, nullptr);
		std::string utf8_str(utf8_size, '\0');
		WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
			utf16_str.length(), &utf8_str[0], utf8_size,
			nullptr, nullptr);

		return utf8_str;
#else
		//TODO: not implemented in other platforms
		return str;
#endif
	}

	std::string encodeURL(const std::string& str)
	{
		if (str.empty()) return str;

		std::stringstream stream;

		for (char c : str)
		{
			if ((c >= '0' && c <= '9') //digit
				|| (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') //alphabet
				|| (c == '-' || c == '_' || c == '.' || c == '~')) //safe characters
				stream << c;
			else
				stream << '%' << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (0xff & c);
		}

		return stream.str();
	}

	std::string getLiveStatusPath(const std::string& channelID)
	{
		return CHZZK_API_PATH_PREFIX_LIVE_STATUS + channelID + CHZZK_API_PATH_SUFFIX_LIVE_STATUS;
	}

	std::string getLiveDetailPath(const std::string& channelID)
	{
		return CHZZK_API_PATH_PREFIX_LIVE_DETAIL + channelID + CHZZK_API_PATH_SUFFIX_LIVE_DETAIL;
	}

	std::string getChannelPath(const std::string& channelID)
	{
		return CHZZK_API_PATH_PREFIX_CHANNEL + channelID;
	}

	std::string getVideoPath(int videoNo)
	{
		return CHZZK_API_PATH_PREFIX_VIDEO + std::to_string(videoNo);
	}

	std::string getRecommendationLivesPath(int size)
	{
		std::string ret = CHZZK_API_PATH_PREFIX_RECOMMENDATION_LIVES;

		ret += "size=" + std::to_string(size);

		return ret;
	}

	std::string getRecommendationLivesPath(const std::string& keyword, int size)
	{
		std::string ret = CHZZK_API_PATH_PREFIX_RECOMMENDATION_LIVES;

		ret += "keyword=" + keyword;
		ret += "&size=" + std::to_string(size);

		return ret;
	}

	std::string getSearchChannelPath(const std::string& keyword, int offset, int size, bool withFirstChannelContent)
	{
		std::string ret = CHZZK_API_PATH_PREFIX_SEARCH_CHANNEL;

		ret += "keyword=" + keyword;
		ret += "&offset=" + std::to_string(offset);
		ret += "&size=" + std::to_string(size);
		ret += "&withFirstChannelContent" + std::to_string(withFirstChannelContent);

		return ret;
	}

	std::string getSearchLivePath(const std::string& keyword, int offset, int size)
	{
		std::string ret = CHZZK_API_PATH_PREFIX_SEARCH_LIVE;

		ret += "keyword=" + keyword;
		ret += "&offset=" + std::to_string(offset);
		ret += "&size=" + std::to_string(size);

		return ret;
	}

	std::string getSearchVideoPath(const std::string& keyword, int offset, int size)
	{
		std::string ret = CHZZK_API_PATH_PREFIX_SEARCH_VIDEO;

		ret += "keyword=" + keyword;
		ret += "&offset=" + std::to_string(offset);
		ret += "&size=" + std::to_string(size);

		return ret;
	}

	std::string getAccessTokenPath(const std::string& chatChannelID, const std::string& chatType)
	{
		std::string ret = CHZZK_API_PATH_PREFIX_ACCESS_TOKEN;

		ret += "channelId=" + chatChannelID;
		ret += "&chatType=" + chatType;

		return ret;
	}

	template <>
	ChzzkUserData parse(nlohmann::json json)
	{
		ChzzkUserData userData;

		json_safe_get(userData.hasProfile, json, "hasProfile");
		json_safe_get(userData.userIDHash, json, "userIdHash");
		json_safe_get(userData.nickname, json, "nickname");
		json_safe_get(userData.profileImageUrl, json, "profileImageUrl");

		if (json.find("penalties") != json.end())
		{
			for (auto& element : json["penalties"])
				userData.penalties.push_back(json_safe_get<std::string>(element));
		}

		json_safe_get(userData.officialNotiAgree, json, "officialNotiAgree");
		json_safe_get(userData.officialNotiAgreeUpdatedDate, json, "officialNotiAgreeUpdateDate");
		json_safe_get(userData.verified, json, "verifiedMask");
		json_safe_get(userData.loggedIn, json, "loggedIn");

		return userData;
	}

	template <>
	ChzzkAccessTokenTemporaryRestrict parse(nlohmann::json json)
	{
		ChzzkAccessTokenTemporaryRestrict temp;

		json_safe_get(temp.createdTime, json, "createdTime");
		json_safe_get(temp.duration, json, "duration");
		json_safe_get(temp.temporaryRestrict, json, "temporaryRestrict");
		json_safe_get(temp.times, json, "times");

		return temp;
	}

	template <>
	ChzzkAccessToken parse(nlohmann::json json)
	{
		ChzzkAccessToken token;

		json_safe_get(token.accessToken, json, "accessToken");
		json_safe_get(token.extraToken, json, "extraToken");
		json_safe_get(token.realNameAuth, json, "realNameAuth");

		if (json.find("temporaryRestrict") != json.end()) token.temporaryRestrict = parse<ChzzkAccessTokenTemporaryRestrict>(json["temporaryRestrict"]);

		return token;
	}

	template <>
	ChzzkChannelFollowingInfo parse(nlohmann::json json)
	{
		ChzzkChannelFollowingInfo info;

		if (json.empty()) info.available = false;
		else
		{
			info.available = true;
			json_safe_get(info.following, json, "following");
			json_safe_get(info.notification, json, "notification");
			json_safe_get(info.followDate, json, "followDate");
		}

		return info;
	}

	template <>
	ChzzkChannelPersonalData parse(nlohmann::json json)
	{
		ChzzkChannelPersonalData data;

		if (json.empty()) data.available = false;
		else
		{
			data.available = true;
			if (json.find("following") != json.end()) data.followInfo = parse<ChzzkChannelFollowingInfo>(json["following"]);
			json_safe_get(data.privateUserBlock, json, "privateUserBlock");
		}

		return data;
	}

	template <>
	ChzzkSubscriptionPaymentAvailability parse(nlohmann::json json)
	{
		ChzzkSubscriptionPaymentAvailability payment;

		json_safe_get(payment.iapAvailability, json, "iapAvailability");
		json_safe_get(payment.iabAvailability, json, "iabAvailability");
		return payment;
	}

	template <>
	ChzzkChannelInfo parse(nlohmann::json json)
	{
		ChzzkChannelInfo info;

		json_safe_get(info.ID, json, "channelId");
		json_safe_get(info.name, json, "channelName");
		json_safe_get(info.imageURL, json, "channelImageUrl");
		json_safe_get(info.verified, json, "verifiedMask");
		json_safe_get(info.description, json, "channelDescription");
		json_safe_get(info.followerCount, json, "followerCount");
		if (json.find("personalData") != json.end()) info.personalData = parse<ChzzkChannelPersonalData>(json["personalData"]);

		return info;
	}

	template <>
	ChzzkChannel parse(nlohmann::json json)
	{
		ChzzkChannel channel;

		json_safe_get(channel.ID, json, "channelId");
		json_safe_get(channel.name, json, "channelName");
		json_safe_get(channel.imageURL, json, "channelImageUrl");
		json_safe_get(channel.verified, json, "verifiedMask");
		json_safe_get(channel.description, json, "channelDescription");
		json_safe_get(channel.followerCount, json, "followerCount");
		if (json.find("personalData") != json.end()) channel.personalData = parse<ChzzkChannelPersonalData>(json["personalData"]);

		json_safe_get(channel.type, json, "channelType");
		json_safe_get(channel.openLive, json, "openLive");
		json_safe_get(channel.subscriptionAvailability, json, "subscriptionAvailability");
		if (json.find("sucbscriptionPaymentAvailability") != json.end()) channel.subscriptionPaymentAvailability = parse<ChzzkSubscriptionPaymentAvailability>(json["sucbscriptionPaymentAvailability"]);
		json_safe_get(channel.adMonetizationAvailability, json, "adMonetizationAvailability");

		return channel;
	}

	template <>
	ChzzkLivePollingStatus parse(nlohmann::json json)
	{
		ChzzkLivePollingStatus status;

		json_safe_get(status.status, json, "status");
		json_safe_get(status.isPublishing, json, "isPublishing");
		json_safe_get(status.playableStatus, json, "playableStatus");
		json_safe_get(status.trafficThrottling, json, "trafficThrottling");
		json_safe_get(status.callPeriodMilliSecond, json, "callPeriodMilliSecond");

		return status;
	}

	template <>
	ChzzkLiveStatus parse(nlohmann::json json)
	{
		ChzzkLiveStatus status;

		json_safe_get(status.title, json, "liveTitle");
		json_safe_get(status.status, json, "status");
		json_safe_get(status.concurrentUserCount, json, "concurrentUserCount");
		json_safe_get(status.accumulatedUserCount, json, "accumulateCount");
		json_safe_get(status.paidPromotion, json, "paidPromotion");
		json_safe_get(status.adult, json, "adult");
		json_safe_get(status.krOnlyViewing, json, "krOnlyViewing");
		json_safe_get(status.clipActive, json, "clipActive");
		json_safe_get(status.chatChannelID, json, "chatChannelId");

		if (json.find("tags") != json.end())
		{
			for (auto& element : json["tags"])
				status.tags.push_back(json_safe_get<std::string>(element));
		}

		json_safe_get(status.categoryType, json, "categoryType");
		json_safe_get(status.liveCategory, json, "liveCategory");
		json_safe_get(status.liveCategoryValue, json, "liveCategoryValue");
		if (json.find("livePollingStatusJson") != json.end()) status.livePollingStatus = parse<ChzzkLivePollingStatus>(json["livePollingStatusJson"]);
		json_safe_get(status.faultStatus, json, "faultStatus");
		json_safe_get(status.userAdultStatus, json, "userAdultStatus");
		json_safe_get(status.blindType, json, "blindType");
		json_safe_get(status.chatActive, json, "chatActive");
		json_safe_get(status.chatAvailableGroup, json, "chatAvailableGroup");
		json_safe_get(status.chatAvailableCondition, json, "chatAvailableCondition");
		json_safe_get(status.minFollowerMinute, json, "minFollowerMinute");
		json_safe_get(status.chatDonationRankingExposure, json, "chatDonationRankingExposure");

		return status;
	}

	template <>
	ChzzkLiveInfo parse(nlohmann::json json)
	{
		ChzzkLiveInfo info;

		json_safe_get(info.ID, json, "liveId");
		json_safe_get(info.title, json, "liveTitle");
		json_safe_get(info.imageURL, json, "liveImageUrl");
		json_safe_get(info.defaultThumbnailImageURL, json, "defaultThumbnailImageUrl");
		json_safe_get(info.concurrentUserCount, json, "concurrentUserCount");
		json_safe_get(info.accumulatedUserCount, json, "accumulateCount");
		json_safe_get(info.openDate, json, "openDate");
		json_safe_get(info.adult, json, "adult");

		if (json.find("tags") != json.end())
		{
			for (auto& element : json["tags"])
				info.tags.push_back(json_safe_get<std::string>(element));
		}

		json_safe_get(info.categoryType, json, "categoryType");
		json_safe_get(info.liveCategory, json, "liveCategory");
		json_safe_get(info.liveCategoryValue, json, "liveCategoryValue");
		json_safe_get(info.channelID, json, "channelId");
		json_safe_get(info.blindType, json, "blindType");
		if (json.find("channel") != json.end()) info.channelInfo = parse<ChzzkChannelInfo>(json["channel"]);

		return info;
	}

	template <>
	ChzzkLive parse(nlohmann::json json)
	{
		ChzzkLive live;

		json_safe_get(live.ID, json, "liveId");
		json_safe_get(live.title, json, "liveTitle");
		json_safe_get(live.imageURL, json, "liveImageUrl");
		json_safe_get(live.defaultThumbnailImageURL, json, "defaultThumbnailImageUrl");
		json_safe_get(live.concurrentUserCount, json, "concurrentUserCount");
		json_safe_get(live.accumulatedUserCount, json, "accumulateCount");
		json_safe_get(live.openDate, json, "openDate");
		json_safe_get(live.adult, json, "adult");

		if (json.find("tags") != json.end())
		{
			for (auto& element : json["tags"])
				live.tags.push_back(json_safe_get<std::string>(element));
		}

		json_safe_get(live.categoryType, json, "categoryType");
		json_safe_get(live.liveCategory, json, "liveCategory");
		json_safe_get(live.liveCategoryValue, json, "liveCategoryValue");
		json_safe_get(live.channelID, json, "channelId");
		json_safe_get(live.blindType, json, "blindType");
		//live.channelInfo is not here

		json_safe_get(live.chatChannelID, json, "chatChannelId");
		json_safe_get(live.livePlayback, json, "livePlaybackJson");
		return live;
	}

	template <>
	ChzzkLiveDetail parse(nlohmann::json json)
	{
		ChzzkLiveDetail live;

		json_safe_get(live.title, json, "liveTitle");
		json_safe_get(live.imageURL, json, "liveImageUrl");
		json_safe_get(live.defaultThumbnailImageURL, json, "defaultThumbnailImageUrl");
		json_safe_get(live.concurrentUserCount, json, "concurrentUserCount");
		json_safe_get(live.accumulatedUserCount, json, "accumulateCount");
		json_safe_get(live.openDate, json, "openDate");
		json_safe_get(live.ID, json, "liveId");
		json_safe_get(live.adult, json, "adult");

		if (json.find("tags") != json.end())
		{
			for (auto& element : json["tags"])
				live.tags.push_back(json_safe_get<std::string>(element));
		}

		json_safe_get(live.chatChannelID, json, "chatChannelId");
		json_safe_get(live.categoryType, json, "categoryType");
		json_safe_get(live.liveCategory, json, "liveCategory");
		json_safe_get(live.liveCategoryValue, json, "liveCategoryValue");
		json_safe_get(live.channelID, json, "channelId");
		json_safe_get(live.livePlayback, json, "livePlaybackJson");
		json_safe_get(live.blindType, json, "blindType");
		if (json.find("channel") != json.end()) live.channelInfo = parse<ChzzkChannelInfo>(json["channel"]);

		json_safe_get(live.status, json, "status");
		json_safe_get(live.closeDate, json, "closeDate");
		json_safe_get(live.clipActive, json, "clipActive");
		json_safe_get(live.chatActive, json, "chatActive");
		json_safe_get(live.chatAvailableGroup, json, "chatAvailableGroup");
		json_safe_get(live.chatAvailableCondition, json, "chatAvailableCondition");
		json_safe_get(live.minFollowerMinute, json, "minFollowerMinute");

		if (json.find("p2pQuality") != json.end())
		{
			for (auto& element : json["p2pQuality"])
				live.p2pQuality.push_back(json_safe_get<std::string>(element));
		}

		json_safe_get(live.userAdultStatus, json, "userAdultStatus");
		json_safe_get(live.chatDonationRankingExposure, json, "chatDonationRankingExposure");
		
		if (json.find("adParameter") != json.end())
			live.adParameter = json["adParameter"].dump();

		return live;
	}

	template <>
	ChzzkVideoInfo parse(nlohmann::json json)
	{
		ChzzkVideoInfo info;
		json_safe_get(info.videoNo, json, "videoNo");
		json_safe_get(info.ID, json, "videoId");
		json_safe_get(info.title, json, "videoTitle");
		json_safe_get(info.type, json, "videoType");
		json_safe_get(info.publishDate, json, "publishDate");
		json_safe_get(info.thumbnailImageURL, json, "thumbnailImageUrl");
		json_safe_get(info.duration, json, "duration");
		json_safe_get(info.readCount, json, "readCount");
		json_safe_get(info.channelID, json, "channelId");
		json_safe_get(info.publishDateAt, json, "publishDateAt");
		json_safe_get(info.adult, json, "adult");
		json_safe_get(info.categoryType, json, "categoryType");
		json_safe_get(info.videoCategory, json, "videoCategory");
		json_safe_get(info.videoCategoryValue, json, "videoCategoryValue");
		json_safe_get(info.blindType, json, "blindType");
		//info.channelInfo is not here

		return info;
	}

	template <>
	ChzzkVideo parse(nlohmann::json json)
	{
		ChzzkVideo video;
		json_safe_get(video.videoNo, json, "videoNo");
		json_safe_get(video.ID, json, "videoId");
		json_safe_get(video.title, json, "videoTitle");
		json_safe_get(video.type, json, "videoType");
		json_safe_get(video.publishDate, json, "publishDate");
		json_safe_get(video.thumbnailImageURL, json, "thumbnailImageUrl");
		json_safe_get(video.duration, json, "duration");
		json_safe_get(video.readCount, json, "readCount");
		json_safe_get(video.channelID, json, "channelId");
		json_safe_get(video.publishDateAt, json, "publishDateAt");
		json_safe_get(video.adult, json, "adult");
		json_safe_get(video.categoryType, json, "categoryType");
		json_safe_get(video.videoCategory, json, "videoCategory");
		json_safe_get(video.videoCategoryValue, json, "videoCategoryValue");
		json_safe_get(video.blindType, json, "blindType");
		if (json.find("channel") != json.end()) video.channelInfo = parse<ChzzkChannelInfo>(json["channel"]);

		json_safe_get(video.exposure, json, "exposure");
		json_safe_get(video.clipActive, json, "clipActive");
		json_safe_get(video.inKey, json, "inKey");
		json_safe_get(video.liveOpenDate, json, "liveOpenDate");
		json_safe_get(video.vodStatus, json, "vodStatus");

		if (json.find("prevVideo") != json.end())
			video.prevVideo = json["prevVideo"].dump();

		if (json.find("nextVideo") != json.end())
			video.nextVideo = json["nextVideo"].dump();

		json_safe_get(video.userAdultStatus, json, "userAdultStatus");
		
		if (json.find("adParameter") != json.end())
			video.adParameter = json["adParameter"].dump();

		return video;
	}
}