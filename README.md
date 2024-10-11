# Chzzkpp
Unofficial C++ implementation of Chzzk API library.

Chzzk의 비공식 API의 C++ 구현 라이브러리입니다.



------

### Dependencies

- [libcurl](https://curl.se/download/) (with ENABLE_WEBSOCKETS)
- [nlohmann/json](https://github.com/nlohmann/json)



------

### Features

현재 구현된 기능들은 다음과 같습니다.

- 검색 (채널, 라이브, 영상)
- 추천 채널/라이브 (사이드바의 추천 채널 혹은 파트너 채널, 메인 페이지의 추천 채널, 시청자 수 상위 방송 등)
- 라이브 정보 조회
- 채널 정보 조회
- 채팅
  - 채팅 메시지 (도네이션, 구독 메시지 포함) 조회
  - 채팅 보내기 ( **테스트는 안 해봤음** )



------

### Build

dependency가 복잡하고, 알못 이슈로 인해 별도의 빌드를 제공하지 않습니다...

1] 그냥 프로젝트에 통째로 source와 header를 포함하여 빌드하거나,

2] 해당 파일들을 library로 한 번 빌드하여 그 라이브러리를 다시 포함해서 쓰시면 됩니다



간략하게 정리하면 다음과 같은 설정이 필요합니다.

- include directories: libcurl의 include directory, nlohmann/json의 include directory
- linking directories: libcurl의 lib directory, 사용하는 SSL의 lib directory



채팅에 Websocket 기능이 사용되므로, libcurl은 ENABLE_WEBSOCKETS를 정의하여 컴파일한 라이브러리를 사용해주세요.



예시) 이 사람이 빌드 및 테스트한 환경

- Windows 10, Visual Studio 2022, C++17

- libcurl 8.7.0 - openSSL 사용, ENABLE_WEBSOCKETS=ON
- linking libraries: libcurl.lib (디버그인 경우 libcurld.lib), libcrypto.lib, libssl.lib
- static build인 경우, 전처리기에 CURL_STATICLIB를 정의해야 함



------

### Note

- 최신 libcurl 패키지에서는 windows용 visual studio project를 찾을 수 없었습니다.

  어떻게 프로젝트를 생성해서 컴파일해도 에러가 나서, 부득이하게 이전 버전을 이용해 테스트했습니다.

- libcurl에서 websocket 관련 기능을 지원한지 얼마 되지 않아서, 아직 experimental이라고 합니다.

  유의하시기 바랍니다. 다른 라이브러리로 구현한 버전도 만드려 했는데, 너무 무겁거나 에러가 나거나 해서 포기했습니다.

- 미구현한 기능이 있습니다.
  - 유저 프로필 조회
  - 채팅 이벤트 파싱 (현재 raw json string을 그대로 제공)
  - 방송 관리 (채팅 블라인드 등)
  
- UTF-8 인코딩을 사용하기 때문에, 특히 한글을 사용하실 때 유의해주세요.
  - std::string str = "가나다"; // invalid keyword
  - std::string str = u8"가나다"; //valid utf-8 keyword

- 로그인에 사용하는 NID_AUT과 NID_SES는 네이버로 로그인된 브라우저의 Application > Cookies > https://chzzk.naver.com 에서 조회할 수 있습니다. (Google Chrome 기준)

- 미션 도네이션의 경우, ChzzkChatEvent::DONATION에서는 미션의 등록 여부만을 알 수 있습니다.

  미션 상태가 변경되는 경우 (수락, 취소, 만료, 성공, 실패 등)는 ChzzkChatEvent::EVENT에서

  type="DONATION_MISSION_IN_PROGRESS" 인자와 함께 수신받을 수 있습니다.



------

### Example

```c++
#include <iostream>

#include <chzzkpp/ChzzkCore.h>
#include <chzzkpp/ChzzkClient.h>
#include <chzzkpp/ChzzkChat.h>

#include <chzzkpp/ChzzkTypes.h>

#ifdef _WIN32
#include <Windows.h>
#endif

void updateMissions(chzzkpp::ChzzkChat* chat, std::unordered_map<std::string, chzzkpp::ChzzkMissionInfo>& missions)
{
	auto client = chat->getClient();
	auto missionResult = client->getMissions(chat->getCurrentChatOptions().channelID);

	missions.clear();

	for (auto& mission : missionResult.missions)
		if (mission.status == chzzkpp::ChzzkMissionStatus::APPROVED)
			missions[mission.ID] = mission;
}

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8); //set cout to print utf8
#endif

	chzzkpp::ChzzkCore* chzzk = new chzzkpp::ChzzkCore();
	//chzzk->setAuth("NID_AUT", "NID_SES");

	chzzkpp::ChzzkClient client(chzzk);

	auto recommendation = client.getTopViewerLives(20);

	for (auto& live_info : recommendation.lives)
	{
		std::cout << u8"방송 제목: " << live_info.title << u8", 채널명: " << live_info.channelInfo.name << u8", 시청자 수: " << live_info.concurrentUserCount << std::endl;
	}

	auto channel = client.searchChannel(u8"스트리머 닉네임").channels[0];
	auto live_detail = client.getLiveDetail(channel.ID);

	std::cout << std::endl << std::endl;

	std::cout << u8"채널 이름: " << channel.name << std::endl;
	std::cout << u8"방송 제목: " << live_detail.title << std::endl;
	std::cout << u8"라이브 여부: " << live_detail.status << std::endl;
	if (live_detail.status == "OPEN") std::cout << u8"시청자 수: " << live_detail.concurrentUserCount << std::endl;

	chzzkpp::ChzzkChatOptions option;
	option.channelID = "스트리머 라이브 ID"; //chzzk.naver.com/live/***

	chzzkpp::ChzzkChat* chat = new chzzkpp::ChzzkChat(&client, option, 5);

	try
	{
		chat->connect();

		chat->addHandler(chzzkpp::ChzzkChatEvent::CHAT, [](auto& str) {
			auto json = nlohmann::json::parse(str);

			std::string nickname = json.find("profile") != json.end() ? json["profile"]["nickname"] : u8"익명";
			std::string message = json["message"];

			std::cout << nickname << ": " << message << std::endl;
		});

		std::unordered_map<std::string, chzzkpp::ChzzkMissionInfo> current_missions;

		chat->addHandler(chzzkpp::ChzzkChatEvent::CONNECT, [&](auto& str) {
			//update current missions
			updateMissions(chat, current_missions);
		});

		size_t handlerID = chat->addHandler(chzzkpp::ChzzkChatEvent::DONATION, [&](auto& str) {
			auto json = nlohmann::json::parse(str);

			std::string nickname = "";
			if (json["extras"]["isAnonymous"] == true)
				nickname = u8"(익명의 후원자)";
			else nickname = json["extras"]["nickname"];

			int amount = json["extras"]["payAmount"];
			std::string message = json["message"];

			std::string donationType = json["extras"]["donationType"];
			
			if (donationType == "CHAT") donationType = u8"채팅";
			else if (donationType == "VIDEO") donationType = u8"영상";
			else if (donationType == "MISSION") donationType = u8"미션";
			else if (donationType == "MISSION_PARTICIPATION")
				std::cout << u8"[" << nickname << u8"님께서 " << amount << u8"원의 상금을 미션에 추가하셨습니다!] " << message << std::endl;
            else
            {
                //unknown
                std::cout << json.dump(4) << std::endl;
            }

            if (donationType == u8"채팅" || donationType == u8"영상" || donationType == u8"미션")
                std::cout << u8"[" << nickname << u8"님의 " << amount << u8"원 " + donationType + (donationType != u8"미션" ? u8" 후원!] " : u8" 등록!] ") << message << std::endl;
		});

		chat->addHandler(chzzkpp::ChzzkChatEvent::SUBSCRIPTION, [](auto& str) {
			auto json = nlohmann::json::parse(str);
			
			std::string nickname = json["extras"]["nickname"];
			int month = json["extras"]["month"];
			std::string tierName = json["extras"]["tierName"];
			//int tierNo = json["extras"]["tierNo"];

			std::string message = json["message"];

			std::cout << u8"[" << nickname << u8"님의 " << month << u8"개월 " << tierName << u8"구독!] " << message << std::endl;
		});

		chat->addHandler(chzzkpp::ChzzkChatEvent::NOTICE, [](auto& str) {
			if (str.empty()) return;

			auto json = nlohmann::json::parse(str);

			std::string message = json["message"];
			std::string nickname = json["extras"]["registerProfile"]["nickname"];
			std::string userTitle = json["extras"]["registerProfile"]["title"]["name"]; //or, you can use: userRoleCode = json["extras"]["registerProfile"]["userRoleCode"];
			std::cout << u8"[현재 고정된 메시지] (" << userTitle << ") " << nickname << ": " << message << std::endl;
		});

		chat->addHandler(chzzkpp::ChzzkChatEvent::EVENT, [&](auto& str) {
			if (str.empty()) return;

			auto json = nlohmann::json::parse(str);

			std::string type = json["type"];

			if (type == chzzkpp::ChzzkEventType::DONATION_MISSION_IN_PROGRESS)
			{
				std::string status = json["status"];

				if (status == chzzkpp::ChzzkMissionStatus::COMPLETED)
				{
					bool success = json["success"];

					std::string nickname = json["isAnonymous"] ? u8"(익명의 후원자)" : json["nickname"];

					int totalPayAmount = json["totalPayAmount"]; //total amount including mission participation (funding)

					std::cout << u8"[" << nickname << u8"님의 미션 " << (success ? u8"성공!] " : u8"실패...] ");
					
					std::string id = json["missionDonationId"];

					if (!success)
					{
						if (current_missions.find(id) != current_missions.end())
							totalPayAmount = totalPayAmount * current_missions[id].failCheeringRate / 100.f;
						else
						{
							auto missionSetting = client.getMissionDonationSetting(chat->getCurrentChatOptions().channelID);

							totalPayAmount = totalPayAmount * missionSetting.failCheeringRate / 100.f;f;
						}
					}

					std::cout << totalPayAmount << u8"원 획득" << std::endl;

					current_missions.erase(id);
				}
				else
				{
					std::cout << u8"미션 상태가 업데이트되었습니다: " << status << std::endl;
					updateMissions(chat, current_missions);
				}
			}
            else if (type == chzzkpp::ChzzkEventType::DONATION_MISSION_PARTICIPATION)
            {
                //you can check particiation mission (funding) info here (participated or completed)
            }
		});

		//chat->removeHandler(chzzkpp::ChzzkChatEvent::DONATION, handlerID); //if you want to remove the handler

		while (!chat->isChatConnected())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

		std::string line;

		while (true)
		{
			std::cout << "Input your command: ";
			std::getline(std::cin, line);

			if (line == "quit")
			{
				chat->close();
				break;
			}
			else if (line == "recent")
			{
				chat->requestRecentChat(10);
			}
			else
				std::cout << "invalid command." << std::endl;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	delete chzzk;
	delete chat;

	return 0;
}
```



------

### Reference

- https://github.com/kimcore/chzzk
- https://github.com/ssogari-dev/Chzzk_Tool/blob/main/Chzzk-API.md



