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

	ChzzkRecommendResult ChzzkClient::getRecommendationLives(int size)
	{
		ChzzkRecommendResult result;
		result.keyword = "";
		result.requested_size = size;
		result.offset = 0;

		auto content = getContent(core->getRecommendationLives(size));

		for (auto& element : content["data"])
			result.lives.push_back(parse<ChzzkLiveInfo>(element));

		return result;
	}

	ChzzkRecommendResult ChzzkClient::getRecommendationLives(const std::string& keyword, int size)
	{
		ChzzkRecommendResult result;
		result.keyword = keyword;
		result.requested_size = size;
		result.offset = 0;

		auto content = getContent(core->getRecommendationLives(keyword, size));

		for (auto& element : content["data"])
			result.lives.push_back(parse<ChzzkLiveInfo>(element));

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

	ChzzkCore* ChzzkClient::getCore()
	{
		return core;
	}
}