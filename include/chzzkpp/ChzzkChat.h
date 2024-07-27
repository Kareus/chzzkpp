#pragma once
#ifndef _CHZZK_CHAT_
#define _CHZZK_CHAT_

#include "Config.h"

#if _USE_CURL
#include <curl/curl.h>
#endif

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <map>
#include <nlohmann/json.hpp>

#include "ChzzkClient.h"

namespace chzzkpp
{
	enum class ChatCommand
	{
		PING = 0,
		PONG = 10000,
		CONNECT = 100,
		CONNECTED = 10100,
		REQUEST_RECENT_CHAT = 5101,
		RECENT_CHAT = 15101,
		EVENT = 93006,
		CHAT = 93101,
		DONATION = 93102,
		KICK = 94005,
		BLOCK = 94006,
		BLIND = 94008,
		NOTICE = 94010,
		PENALTY = 94015,
		SEND_CHAT = 3101
	};

	enum class ChatType
	{
		NONE = 0,
		TEXT = 1,
		IMAGE = 2,
		STICKER = 3,
		VIDEO = 4,
		RICH = 5,
		DONATION = 10,
		SUBSCRIPTION = 11,
		SYSTEM_MESSAGE = 30
	};

	//TODO: maybe we can parse event json into struct...
	enum class ChzzkChatEvent
	{
		CONNECT,		//chat connect event. argument: empty
		RECONNECT,		//chat reconnect event. argument: chatChannelID string
		DISCONNECT,		//chat disconnect event. argument: chatChannelID string
		NOTICE,			//chat notice event. argument: notice message json
		CHAT,			//chat event. argument: chat message json
		DONATION,		//donation event. argument: donation message json
		SUBSCRIPTION,	//subscription event. argument: subscription message json
		SYSTEM_MESSAGE,	//system message event. argument: system message json
		BLIND			//chat blind event. argument: blind alert message?
	};

	struct ChzzkChatOptions
	{
		std::string chatChannelID;
		std::string accessToken;
		std::string channelID;
		int pollTime;

		const static int DEFAULT_POLL_TIME = 30 * 1000;

		ChzzkChatOptions() : chatChannelID(""), accessToken(""), channelID(""), pollTime(DEFAULT_POLL_TIME)
		{
		}
	};


	//TODO: not sure about multithread safety
	class ChzzkChat
	{
#if _USE_CURL
		CURL* curl;
		std::thread receiverThread;
		std::mutex receiverMutex;

		//message receive loop for libcurl
		//if you are implementing with other websocket libraries, you would not have to implement this function if the library supports async callback or is already multi-threaded
		void _receive();
#endif

		//// library specific functions
		////

		void _connect();
		void _close();
		void _send(const std::string& message);
		
		////
		////


		ChzzkClient* client;
		ChzzkChatOptions option;
		int timeout;
		std::string sid;
		std::string uid;

		//is there good alternative container type for this?
		std::map<ChzzkChatEvent, std::map<size_t, std::function<void(const std::string&)>>> handlers;

		static const int PING_TIME = 20 * 1000;

		std::atomic<bool> connected;
		std::string ws_path;
		nlohmann::json _default;
		bool reconnecting;

		bool isPolling;
		std::thread chatChannelIDThread;

		bool isPinging;
		std::thread pingTimerThread;

		nlohmann::json parseChat(const nlohmann::json& json, bool isRecent = false);
		void onOpen();
		void onMessage(const std::string& message);
		void onClose();

		void reconnect();
		void updateChatChannelID();
		void startPolling();
		void stopPolling();
		void sendPing();
		void startPing();
		void stopPing();

		void call(ChzzkChatEvent type, const std::string& message);

	public:
		//@timeout connection timeout seconds. never times out if value is 0
		ChzzkChat(ChzzkClient* client, ChzzkChatOptions option, int timeout = 0);
		~ChzzkChat();

		void connect();
		void close();

		//@timeout connection timeout seconds. never times out if value is 0
		void setConnectionTimeout(int timeout);
		int getConnectionTimeout() const;

		size_t addHandler(ChzzkChatEvent type, const std::function<void(const std::string&)>& func);
		void removeHandler(ChzzkChatEvent type, size_t id);
		void removeHandlers(ChzzkChatEvent type);
		void removeAllHandlers();

		void requestRecentChat(int size = 50);
		//TODO: sendChat NOT tested yet!!
		void sendChat(const std::string& message, const std::map<std::string, std::string>& emojis = {});

		ChzzkChatOptions& getCurrentChatOptions();

		const ChzzkChatOptions& getCurrentChatOptions() const;

		bool isConnected() const;
	};
}

#endif