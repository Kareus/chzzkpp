#include <chzzkpp/ChzzkChat.h>
#include <chzzkpp/Path.h>
#include <chzzkpp/ChzzkUtils.h>

#if _DEBUG
#include <iostream>
#endif

namespace chzzkpp
{
	///////////////////////////
	///////////////////////////
	//// libcurl implementation

	ChzzkChat::ChzzkChat(ChzzkClient* client, ChzzkChatOptions option, int timeout) : client(client), option(option), sid(""), uid(""), connected(false), reconnecting(false), isPolling(false), isPinging(false), timeout(timeout)
	{
		//initialize the curl
		curl = curl_easy_init();

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 2L);
	}

	ChzzkChat::~ChzzkChat()
	{
		if (connected) _close();
		removeAllHandlers();

		curl_easy_cleanup(curl);
		curl = nullptr;
	}

	void ChzzkChat::_receive()
	{
		size_t len;
		const struct curl_ws_frame* meta;
		std::string message;

		while (connected)
		{
			{
				std::lock_guard<std::mutex> guard(receiverMutex);
				if (!connected) break;

				char buffer[256];
				CURLcode res = curl_ws_recv(curl, buffer, 255, &len, &meta);

				if (res == CURLE_OK)
				{
					buffer[len] = 0;
					message += buffer;

					if (!meta->bytesleft)
					{
						onMessage(message.data());
						message.clear();
					}
				}
				else if (res == CURLE_GOT_NOTHING) continue;
				else if (res != CURLE_AGAIN)
				{
					static const std::string ERR_MSG = "Error occured receiving message: ";
					throw std::exception((ERR_MSG + curl_easy_strerror(res)).c_str());
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}


	void ChzzkChat::_connect()
	{
		//connect to the chat
		if (connected) throw std::exception("Chat is already connected.");

		std::lock_guard<std::mutex> guard(receiverMutex);

		curl_easy_setopt(curl, CURLOPT_URL, ws_path.c_str());
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK)
			throw std::exception(curl_easy_strerror(res));
		else
			onOpen();
	}

	void ChzzkChat::_close()
	{
		{
			std::lock_guard<std::mutex> guard(receiverMutex);
			size_t sent;
			curl_ws_send(curl, "", 0, &sent, 0, CURLWS_CLOSE);

			onClose();
		}

		if (receiverThread.joinable()) receiverThread.join();
	}

	void ChzzkChat::_send(const std::string& message)
	{
		size_t len = message.size();
		size_t sent;

		curl_ws_send(curl, message.c_str(), len, &sent, 0, CURLWS_TEXT);
	}

	///////////////////////////
	///////////////////////////
	//// private common methods

	void ChzzkChat::onOpen()
	{
		connected = true;

		nlohmann::json body = {
				{"accTkn", option.accessToken},
				{"auth", uid.empty() ? "READ" : "SEND"},
				{"devType", 2001},
				{"uid", uid}
		};

		nlohmann::json json = {
			{"bdy", body },
			{"cmd", ChatCommand::CONNECT},
			{"tid", 1}
		};
		json.update(_default);

		_send(json.dump());

#if _USE_CURL
		receiverThread = std::thread(&ChzzkChat::_receive, this);
#endif

		if (!reconnecting) startPolling();

	}

	void ChzzkChat::onClose()
	{
		if (!reconnecting)
		{
			call(ChzzkChatEvent::DISCONNECT, option.chatChannelID);

			stopPolling();
			option.chatChannelID = "";
		}

		stopPing();
		sid = "";

		option.accessToken = "";
		uid = "";

		connected = false;
	}

	void ChzzkChat::reconnect()
	{
		reconnecting = true;

		if (connected)
		{
			close();
			connect();
		}
	}

	void ChzzkChat::updateChatChannelID()
	{
		int pollCount = option.pollTime;

		while (connected && isPolling)
		{
			if (pollCount >= option.pollTime)
			{
				auto currentChatChannelID = client->getLiveStatus(option.channelID).chatChannelID;

				if (!currentChatChannelID.empty() && currentChatChannelID != option.chatChannelID)
				{
					option.chatChannelID = currentChatChannelID;

					reconnect();
				}

				pollCount = 0;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			pollCount += 10;
		}
	}

	void ChzzkChat::startPolling()
	{
		if (!option.pollTime || isPolling) return;

		isPolling = true;
		chatChannelIDThread = std::thread(&ChzzkChat::updateChatChannelID, this);
	}

	void ChzzkChat::stopPolling()
	{
		if (isPolling)
		{
			isPolling = false;
			if (chatChannelIDThread.joinable()) chatChannelIDThread.join();
		}
	}

	void ChzzkChat::sendPing()
	{
		int pingCount = 0;

		while (connected && isPinging)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			pingCount += 10;

			if (pingCount >= PING_TIME)
			{
				nlohmann::json json = {
					{"cmd", ChatCommand::PING},
					{"ver", 2}
				};

				_send(json.dump());

				pingCount = 0;
			}

		}
	}

	void ChzzkChat::startPing()
	{
		if (isPinging) stopPing();

		isPinging = true;
		pingTimerThread = std::thread(&ChzzkChat::sendPing, this);
	}

	void ChzzkChat::stopPing()
	{
		isPinging = false;
		if (pingTimerThread.joinable()) pingTimerThread.join();
	}

	nlohmann::json ChzzkChat::parseChat(const nlohmann::json& json, bool isRecent)
	{
		std::string _profile = json_safe_get<std::string>(json["profile"]);
		std::string _extras = json_safe_get<std::string>(json["extras"]);

		auto profile = _profile.empty() ? nlohmann::json() : nlohmann::json::parse(_profile);
		auto extras = _extras.empty() ? nlohmann::json() : nlohmann::json::parse(_extras);

		auto params = extras.is_null() ? nlohmann::json() : extras["params"];
		std::string registerChatProfileJson = params.is_null() ? "" : params["registerChatProfileJson"];
		std::string targetChatProfileJson = params.is_null() ? "" : params["targetChatProfileJson"];

		if (!registerChatProfileJson.empty() && !targetChatProfileJson.empty())
		{
			params["registerChatProfile"] = nlohmann::json::parse(registerChatProfileJson);
			params["targetChatProfile"] = nlohmann::json::parse(targetChatProfileJson);

			params.erase("registerChatProfileJson");
			params.erase("targetChatProfileJson");

			extras["params"] = params;
		}

		std::string message = "";
		if (json.find("msg") != json.end()) message = json["msg"];
		else if (json.find("content") != json.end()) message = json["content"]; //case of recent message

		int memberCount = 0;
		if (json.find("mbrCnt") != json.end()) memberCount = json["mbrCnt"];
		else if (json.find("memberCount") != json.end()) memberCount = json["memberCount"]; //case of recent message

		unsigned long long time = 0;
		if (json.find("msgTime") != json.end()) time = json["msgTime"];
		else if (json.find("messageTime") != json.end()) time = json["messageTime"]; //case of recent message

		std::string messageStatusType = "";
		if (json.find("msgStatusType") != json.end()) messageStatusType = json["msgStatusType"];
		else if (json.find("messageStatusType") != json.end()) messageStatusType = json["messageStatusType"]; //case of recent message

		bool hidden = (messageStatusType == "HIDDEN");

		nlohmann::json parsed = {
			{"profile", profile},
			{"extras", extras},
			{"hidden", hidden},
			{"message", message},
			{"time", time},
			{"isRecent", isRecent}
		};

		if (memberCount) parsed["memberCount"] = memberCount;

		return parsed;
		return extras;
	}

	void ChzzkChat::onMessage(const std::string& message)
	{
		if (message.empty()) return;

		nlohmann::json json;

		try
		{
			json = nlohmann::json::parse(message);
		}
		catch (std::exception& e)
		{
#if _DEBUG
			std::cerr << e.what() << std::endl;
#endif
			return;
		}

		auto body = json["bdy"];
		ChatCommand cmd = json["cmd"];

		switch (cmd)
		{
		case ChatCommand::CONNECTED:
			sid = body["sid"];

			if (reconnecting)
			{
				call(ChzzkChatEvent::RECONNECT, option.chatChannelID);
				reconnecting = false;
			}
			else
				call(ChzzkChatEvent::CONNECT, "");

			break;

		case ChatCommand::PING:
			{
				nlohmann::json json = {
					{"cmd", ChatCommand::PONG},
					{"ver", 2}
				};

				_send(json.dump());
			}
			break;

		case ChatCommand::CHAT:
		case ChatCommand::RECENT_CHAT:
		case ChatCommand::DONATION:
			{
				bool isRecent = (cmd == ChatCommand::RECENT_CHAT);

				auto _notice = body.find("notice") != body.end() ? body["notice"] : nlohmann::json();

				if (!_notice.empty()) //parsing notice message
				{
					auto notice = parseChat(body["notice"], isRecent).dump();
					call(ChzzkChatEvent::NOTICE, notice);
				}

				auto chats = body;

				if (chats.find("messageList") != chats.end()) chats = chats["messageList"]; //recent messages

				for (auto chat : chats)
				{
					ChatType type = ChatType::NONE;

					if (chat.find("msgTypeCode") != chat.end()) type = chat["msgTypeCode"];
					else if (chat.find("messageTypeCode") != chat.end()) type = chat["messageTypeCode"]; //case of recent message

					auto message = parseChat(chat, isRecent).dump();

					switch (type)
					{
					case ChatType::TEXT:
						call(ChzzkChatEvent::CHAT, message);
						break;

					case ChatType::DONATION:
						call(ChzzkChatEvent::DONATION, message);
						break;

					case ChatType::SUBSCRIPTION:
						call(ChzzkChatEvent::SUBSCRIPTION, message);
						break;

					case ChatType::SYSTEM_MESSAGE:
						call(ChzzkChatEvent::SYSTEM_MESSAGE, message);
						break;
					}
				}
			}
			break;

		case ChatCommand::NOTICE:
			call(ChzzkChatEvent::NOTICE, body.empty() || body.is_null() ? "" : parseChat(body));
			break;

		case ChatCommand::BLIND:
			call(ChzzkChatEvent::BLIND, body.is_string() ? (std::string)body : body.dump()); //idk what comes here, so just converting it
			break;
		}

		if (cmd != ChatCommand::PONG)
			startPing();
	}

	void ChzzkChat::call(ChzzkChatEvent type, const std::string& message)
	{
		for (auto& p : handlers[type])
			p.second(message);
	}

	/////////////////////
	/////////////////////
	//// public methods

	void ChzzkChat::connect()
	{
		if (!option.channelID.empty() && option.chatChannelID.empty())
			option.chatChannelID = client->getLiveStatus(option.channelID).chatChannelID;
		
		if (option.chatChannelID.empty())
			throw std::exception("Cannot find the chat... Check your channelID, or the live status is adult. You need to log in to access the adult live chat.");

		if (!option.chatChannelID.empty() && option.accessToken.empty())
		{

			uid = client->getCore()->hasAuth() ? client->getUserData().userIDHash : "";

			option.accessToken = client->getAccessToken(option.chatChannelID).accessToken;
		}

		_default = {
			{"cid", option.chatChannelID},
			{"svcid", "game"},
			{"ver", 2}
		};

		int serverID = 0;

		for (auto c : option.chatChannelID)
			serverID += c;

		serverID = serverID % 9 + 1;

		ws_path = CHZZK_CHAT_WEBSOCKET_PATH_PREFIX + std::to_string(serverID) + CHZZK_CHAT_WEBSOCKET_PATH_SUFFIX;

		_connect();
	}

	void ChzzkChat::close()
	{
		if (!connected) throw std::exception("Chat is not connected.");

		_close();
	}

	void ChzzkChat::setConnectionTimeout(int timeout)
	{
		this->timeout = timeout;
	}

	int ChzzkChat::getConnectionTimeout() const
	{
		return timeout;
	}

	size_t ChzzkChat::addHandler(ChzzkChatEvent type, const std::function<void(const std::string&)>& func)
	{
#if _USE_CURL
		std::lock_guard<std::mutex> guard(receiverMutex);
#endif

		size_t id = 0;

		while (handlers[type].find(id) != handlers[type].end()) id++;

		handlers[type].emplace(id, func);
		return id;
	}

	void ChzzkChat::removeHandler(ChzzkChatEvent type, size_t id)
	{
#if _USE_CURL
		std::lock_guard<std::mutex> guard(receiverMutex);
#endif

		handlers[type].erase(id);
	}

	void ChzzkChat::removeHandlers(ChzzkChatEvent type)
	{
#if _USE_CURL
		std::lock_guard<std::mutex> guard(receiverMutex);
#endif

		handlers[type].clear();
	}

	void ChzzkChat::removeAllHandlers()
	{
#if _USE_CURL
		std::lock_guard<std::mutex> guard(receiverMutex);
#endif

		for (auto& h : handlers)
			h.second.clear();
	}

	void ChzzkChat::requestRecentChat(int size)
	{
		if (!connected)
			throw std::exception("Not Connected to the chat.");

		nlohmann::json bdy = { {"recentMessageCount", size} };

		nlohmann::json json = {
			{"bdy", bdy},
			{"cmd", ChatCommand::REQUEST_RECENT_CHAT},
			{"sid", sid},
			{"tid", 2}
		};

		json.update(_default);
		_send(json.dump());
	}

	void ChzzkChat::sendChat(const std::string& message, const std::map<std::string, std::string>& emojis)
	{
		if (!connected)
			throw std::exception("Chat is not connected.");

		if (uid.empty())
			throw std::exception("Chat Client is not logged in.");

		nlohmann::json emoji_json;

		for (auto& p : emojis)
			emoji_json[p.first] = p.second;

		nlohmann::json extras = {
			{"chatType", "STREAMING"},
			{"emojis", emoji_json},
			{"osType", config::OS_TYPE},
			{"streamingChannelId", option.chatChannelID}
		};

		nlohmann::json body = {
			{"extras", extras.dump()},
			{"msg", message},
			{"msgTime", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()},
			{"msgTypeCode", ChatType::TEXT}
		};

		nlohmann::json json = {
			{"bdy", body},
			{"retry", false},
			{"cmd", ChatCommand::SEND_CHAT},
			{"sid", sid},
			{"tid", 3}
		};

		json.update(_default);

		_send(json.dump());
	}



	ChzzkChatOptions& ChzzkChat::getCurrentChatOptions()
	{
		return option;
	}

	const ChzzkChatOptions& ChzzkChat::getCurrentChatOptions() const
	{
		return option;
	}

	bool ChzzkChat::isConnected() const
	{
		return connected;
	}
}