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
- 추천 영상 (키워드에 해당하는 현재 방송 중인 채널들을 시청자 수 상위 n개 목록을 가져옵니다)
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

using namespace std;

int main()
{
#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8); //set cout to print utf8
#endif
    
	chzzkpp::ChzzkCore* chzzk = new chzzkpp::ChzzkCore();
	//chzzk->setAuth("NID_AUT", "NID_SES"); //로그인

	chzzkpp::ChzzkClient client(chzzk);

    
    //시청자 수 기준 상위 20명의 방송 조회
	auto recommendation = client.getRecommendationLives(20);

	for (auto& live_info : recommendation.lives)
	{
		std::cout << u8"방송 제목: " << live_info.title << u8", 채널명: " << live_info.channelInfo.name
            << u8", 시청자 수: " << live_info.concurrentUserCount << std::endl;
	}
    

    //스트리머 닉네임으로 채널 검색
	auto search_result = client.searchChannel(u8"스트리머 닉네임");

	if (search_result.channels.empty())
		std::cout << u8"검색 결과가 존재하지 않습니다. 다른 키워드로 시도해보세요." << std::endl;
	else
	{
		auto channel = search_result.channels[0];
		auto live_detail = client.getLiveDetail(channel.ID);

		std::cout << std::endl << std::endl;

		std::cout << u8"채널 이름: " << channel.name << std::endl;
		std::cout << u8"방송 제목: " << live_detail.title << std::endl;
		std::cout << u8"라이브 여부: " << live_detail.status << std::endl;
		if (live_detail.status == "OPEN") std::cout << u8"시청자 수: " << live_detail.concurrentUserCount << std::endl;
	}

    
    //채팅 연결
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

		size_t handlerID = chat->addHandler(chzzkpp::ChzzkChatEvent::DONATION, [](auto& str) {
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

			std::cout << u8"[" << nickname << u8"님의 " << amount << u8"원 " + donationType + u8" 후원!] " << message << std::endl;
			});

		chat->addHandler(chzzkpp::ChzzkChatEvent::NOTICE, [](auto& str) {
			if (str.empty()) return;

			auto json = nlohmann::json::parse(str);

			std::string message = json["message"];
			std::string nickname = json["extras"]["registerProfile"]["nickname"];
			std::string userTitle = json["extras"]["registerProfile"]["title"]["name"];
            //or, you can use: std::string userRoleCode = json["extras"]["registerProfile"]["userRoleCode"];
            
			std::cout << u8"[현재 고정된 메시지] (" << userTitle << ") " << nickname << ": " << message << std::endl;
			});

		//chat->removeHandler(chzzkpp::ChzzkChatEvent::DONATION, handlerID); //if you want to remove the handler

		while (!chat->isConnected())
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



