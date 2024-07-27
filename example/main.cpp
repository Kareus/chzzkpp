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
	//chzzk->setAuth("NID_AUT", "NID_SES"); //�α���

	chzzkpp::ChzzkClient client(chzzk);


	//��û�� �� ���� ���� 20���� ��� ��ȸ
	auto recommendation = client.getRecommendationLives(20);

	for (auto& live_info : recommendation.lives)
	{
		std::cout << u8"��� ����: " << live_info.title << u8", ä�θ�: " << live_info.channelInfo.name << u8", ��û�� ��: " << live_info.concurrentUserCount << std::endl;
	}


	//��Ʈ���� �г������� ä�� �˻�
	auto search_result = client.searchChannel(u8"��Ʈ���� �г���");

	if (search_result.channels.empty())
		std::cout << u8"�˻� ����� �������� �ʽ��ϴ�. �ٸ� Ű����� �õ��غ�����." << std::endl;
	else
	{
		auto channel = search_result.channels[0];
		auto live_detail = client.getLiveDetail(channel.ID);

		std::cout << std::endl << std::endl;

		std::cout << u8"ä�� �̸�: " << channel.name << std::endl;
		std::cout << u8"��� ����: " << live_detail.title << std::endl;
		std::cout << u8"���̺� ����: " << live_detail.status << std::endl;
		if (live_detail.status == "OPEN") std::cout << u8"��û�� ��: " << live_detail.concurrentUserCount << std::endl;
	}


	//ä�� ����
	chzzkpp::ChzzkChatOptions option;
	option.channelID = "��Ʈ���� ���̺� ID"; //chzzk.naver.com/live/***

	chzzkpp::ChzzkChat* chat = new chzzkpp::ChzzkChat(&client, option, 5);

	try
	{
		chat->connect();

		chat->addHandler(chzzkpp::ChzzkChatEvent::CHAT, [](auto& str) {
			auto json = nlohmann::json::parse(str);

			std::string nickname = json.find("profile") != json.end() ? json["profile"]["nickname"] : u8"�͸�";
			std::string message = json["message"];

			std::cout << nickname << ": " << message << std::endl;
			});

		size_t handlerID = chat->addHandler(chzzkpp::ChzzkChatEvent::DONATION, [](auto& str) {
			auto json = nlohmann::json::parse(str);

			std::string nickname = "";
			if (json["extras"]["isAnonymous"] == true)
				nickname = u8"(�͸��� �Ŀ���)";
			else nickname = json["extras"]["nickname"];

			int amount = json["extras"]["payAmount"];
			std::string message = json["message"];

			std::string donationType = json["extras"]["donationType"];

			if (donationType == "CHAT") donationType = u8"ä��";
			else if (donationType == "VIDEO") donationType = u8"����";
			else if (donationType == "MISSION") donationType = u8"�̼�";

			std::cout << u8"[" << nickname << u8"���� " << amount << u8"�� " + donationType + u8" �Ŀ�!] " << message << std::endl;
			});

		chat->addHandler(chzzkpp::ChzzkChatEvent::NOTICE, [](auto& str) {
			if (str.empty()) return;

			auto json = nlohmann::json::parse(str);

			std::string message = json["message"];
			std::string nickname = json["extras"]["registerProfile"]["nickname"];
			std::string userTitle = json["extras"]["registerProfile"]["title"]["name"];
			//or, you can use: std::string userRoleCode = json["extras"]["registerProfile"]["userRoleCode"];
			std::cout << u8"[���� ������ �޽���] (" << userTitle << ") " << nickname << ": " << message << std::endl;
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