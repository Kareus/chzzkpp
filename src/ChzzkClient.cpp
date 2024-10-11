#include <chzzkpp/ChzzkClient.h>
#include <chzzkpp/ChzzkUtils.h>

#if _DEBUG
#include <iostream>
#endif

namespace chzzkpp
{
	ChzzkClient::ChzzkClient(ChzzkCore* core) : core(core)
	{

	}

	nlohmann::json ChzzkClient::getContent(const std::string& data)
	{
		if (!nlohmann::json::accept(data))
		{
#if _DEBUG
			std::cerr << "Input data is not JSON object: " << data << std::endl;
#endif
			return nlohmann::json();
		}

		nlohmann::json json = nlohmann::json::parse(data);
		int code = json["code"];

		if (code == 200) return json["content"];
		else
		{
			std::string message = json["message"];
			auto e = invalid_status_exception(code, message);

#if _DEBUG
			std::cerr << e.what() << std::endl;
#endif
			throw e;

			return nlohmann::json();
		}

	}

	ChzzkChannel ChzzkClient::getChannel(const std::string& channelID)
	{
		return parse<ChzzkChannel>(getContent(core->getChannel(channelID)));
	}

	ChzzkLiveStatus ChzzkClient::getLiveStatus(const std::string& channelID)
	{
		return parse<ChzzkLiveStatus>(getContent(core->getLiveStatus(channelID)));
	}

	ChzzkLiveDetail ChzzkClient::getLiveDetail(const std::string& channelID)
	{
		return parse<ChzzkLiveDetail>(getContent(core->getLiveDetail(channelID)));
	}

	ChzzkVideo ChzzkClient::getVideo(int videoNo)
	{
		return parse<ChzzkVideo>(getContent(core->getVideo(videoNo)));
	}

	ChzzkTopViewerResult ChzzkClient::getTopViewerLives(int size)
	{
		ChzzkTopViewerResult result;
		result.keyword = "";
		result.requested_size = size;
		result.offset = 0;

		auto content = getContent(core->getTopViewerLives(size));

		for (auto& element : content["data"])
			result.lives.push_back(parse<ChzzkLiveBase>(element));

		return result;
	}

	ChzzkTopViewerResult ChzzkClient::getTopViewerLives(const std::string& keyword, int size)
	{
		ChzzkTopViewerResult result;
		result.keyword = keyword;
		result.requested_size = size;
		result.offset = 0;

		auto content = getContent(core->getTopViewerLives(keyword, size));

		for (auto& element : content["data"])
			result.lives.push_back(parse<ChzzkLiveBase>(element));

		return result;
	}

	std::vector<ChzzkRecommendChannel> ChzzkClient::getRecommendationChannels()
	{
		std::vector<ChzzkRecommendChannel> result;

		auto content = getContent(core->getRecommendationChannels());

		for (auto& element : content["recommendationChannels"])
			result.push_back(parse<ChzzkRecommendChannel>(element));

		return result;
	}

	std::vector<ChzzkRecommendPartnerChannel> ChzzkClient::getRecommendationPartners()
	{
		std::vector<ChzzkRecommendPartnerChannel> result;

		auto content = getContent(core->getRecommendationChannels(true));

		for (auto& element : content["streamerPartners"])
			result.push_back(parse<ChzzkRecommendPartnerChannel>(element));

		return result;
	}

	ChzzkLiveResult ChzzkClient::getRecommendationLives()
	{
		ChzzkLiveResult result;
		result.keyword = "";
		result.offset = 0;
		result.requested_size = 0;

		auto content = getContent(core->getRecommendationLives());

		for (auto& element : content["topRecommendedLives"])
			result.lives.push_back(parse<ChzzkLive>(element));

		return result;
	}

	ChzzkUserData ChzzkClient::getUserData()
	{
		return parse<ChzzkUserData>(getContent(core->getUserData()));
	}

	ChzzkAccessToken ChzzkClient::getAccessToken(const std::string& chatChannelID)
	{
		return parse<ChzzkAccessToken>(getContent(core->getAccessToken(chatChannelID)));
	}

	ChzzkChannelResult ChzzkClient::searchChannel(const std::string& keyword, int offset, int size, bool withFirstChannelContent)
	{
		ChzzkChannelResult result;
		result.keyword = keyword;
		result.offset = offset;
		result.requested_size = size;

		auto content = getContent(core->searchChannel(keyword, offset, size, withFirstChannelContent));

		for (auto& element : content["data"])
			result.channels.push_back(parse<ChzzkChannelInfo>(element["channel"]));

		return result;
	}

	ChzzkLiveResult ChzzkClient::searchLive(const std::string& keyword, int offset, int size)
	{
		ChzzkLiveResult result;
		result.keyword = keyword;
		result.offset = offset;
		result.requested_size = size;

		auto content = getContent(core->searchLive(keyword, offset, size));

		for (auto& element : content["data"])
		{
			ChzzkLive live = parse<ChzzkLive>(element["live"]);
			live.channelInfo = parse<ChzzkChannelInfo>(element["channel"]);

			result.lives.push_back(live);
		}

		return result;
	}

	ChzzkVideoResult ChzzkClient::searchVideo(const std::string& keyword, int offset, int size)
	{
		ChzzkVideoResult result;
		result.keyword = keyword;
		result.offset = offset;
		result.requested_size = size;

		auto content = getContent(core->searchVideo(keyword, offset, size));

		for (auto& element : content["data"])
		{
			ChzzkVideoInfo video = parse<ChzzkVideoInfo>(element["video"]);
			video.channelInfo = parse<ChzzkChannelInfo>(element["channel"]);

			result.videos.push_back(video);
		}

		return result;
	}

	ChzzkMissionResult ChzzkClient::getMissions(const std::string& channelID, bool mine, int page, int size)
	{
		ChzzkMissionResult result;
		result.page = page;
		result.size = size;

		auto content = getContent(core->getMissions(channelID, mine, page, size));

		result.totalCount = content["totalCount"];
		result.totalPages = content["totalPages"];

		for (auto& element : content["data"])
		{
			ChzzkMissionInfo mission = parse<ChzzkMissionInfo>(element);
			result.missions.push_back(mission);
		}

		return result;
	}

	ChzzkChatDonationSetting ChzzkClient::getChatDonationSetting(const std::string& channelID)
	{
		auto content = getContent(core->getChatDonationSetting(channelID));

		return parse<ChzzkChatDonationSetting>(content);
	}

	ChzzkVideoDonationSetting ChzzkClient::getVideoDonationSetting(const std::string& channelID)
	{
		auto content = getContent(core->getVideoDonationSetting(channelID));

		return parse<ChzzkVideoDonationSetting>(content);
	}

	ChzzkMissionDonationSetting ChzzkClient::getMissionDonationSetting(const std::string& channelID)
	{
		auto content = getContent(core->getMissionDonationSetting(channelID));

		return parse<ChzzkMissionDonationSetting>(content);
	}

	ChzzkCore* ChzzkClient::getCore()
	{
		return core;
	}
}