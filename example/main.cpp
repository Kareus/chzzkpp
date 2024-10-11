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
		std::cout << u8"��� ����: " << live_info.title << u8", ä�θ�: " << live_info.channelInfo.name << u8", ��û�� ��: " << live_info.concurrentUserCount << std::endl;
	}

	auto channel = client.searchChannel(u8"��Ʈ���� �г���").channels[0];
	auto live_detail = client.getLiveDetail(channel.ID);

	std::cout << std::endl << std::endl;

	std::cout << u8"ä�� �̸�: " << channel.name << std::endl;
	std::cout << u8"��� ����: " << live_detail.title << std::endl;
	std::cout << u8"���̺� ����: " << live_detail.status << std::endl;
	if (live_detail.status == "OPEN") std::cout << u8"��û�� ��: " << live_detail.concurrentUserCount << std::endl;

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

		std::unordered_map<std::string, chzzkpp::ChzzkMissionInfo> current_missions;

		chat->addHandler(chzzkpp::ChzzkChatEvent::CONNECT, [&](auto& str) {
			//update current missions
			updateMissions(chat, current_missions);
		});

		size_t handlerID = chat->addHandler(chzzkpp::ChzzkChatEvent::DONATION, [&](auto& str) {
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
			else if (donationType == "MISSION_PARTICIPATION")
				std::cout << u8"[" << nickname << u8"�Բ��� " << amount << u8"���� ����� �̼ǿ� �߰��ϼ̽��ϴ�!] " << message << std::endl;
			else
			{
				//unknown
				std::cout << json.dump(4) << std::endl;
			}

			if (donationType == u8"ä��" || donationType == u8"����" || donationType == u8"�̼�")
				std::cout << u8"[" << nickname << u8"���� " << amount << u8"�� " + donationType + (donationType != u8"�̼�" ? u8" �Ŀ�!] " : u8" ���!] ") << message << std::endl;
		});

		chat->addHandler(chzzkpp::ChzzkChatEvent::SUBSCRIPTION, [](auto& str) {
			auto json = nlohmann::json::parse(str);
			
			std::string nickname = json["extras"]["nickname"];
			int month = json["extras"]["month"];
			std::string tierName = json["extras"]["tierName"];
			//int tierNo = json["extras"]["tierNo"];

			std::string message = json["message"];

			std::cout << u8"[" << nickname << u8"���� " << month << u8"���� " << tierName << u8"����!] " << message << std::endl;
		});

		chat->addHandler(chzzkpp::ChzzkChatEvent::NOTICE, [](auto& str) {
			if (str.empty()) return;

			auto json = nlohmann::json::parse(str);

			std::string message = json["message"];
			std::string nickname = json["extras"]["registerProfile"]["nickname"];
			std::string userTitle = json["extras"]["registerProfile"]["title"]["name"]; //or, you can use: userRoleCode = json["extras"]["registerProfile"]["userRoleCode"];
			std::cout << u8"[���� ������ �޽���] (" << userTitle << ") " << nickname << ": " << message << std::endl;
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

					std::string nickname = json["isAnonymous"] ? u8"(�͸��� �Ŀ���)" : json["nickname"];

					int totalPayAmount = json["totalPayAmount"];

					std::cout << u8"[" << nickname << u8"���� �̼� " << (success ? u8"����!] " : u8"����...] ");
					
					std::string id = json["missionDonationId"];

					if (!success)
					{
						if (current_missions.find(id) != current_missions.end())
							totalPayAmount = totalPayAmount * current_missions[id].failCheeringRate / 100.f;
						else
						{
							auto missionSetting = client.getMissionDonationSetting(chat->getCurrentChatOptions().channelID);

							totalPayAmount = totalPayAmount * missionSetting.failCheeringRate / 100.f;
						}
					}

					std::cout << totalPayAmount << u8"�� ȹ��" << std::endl;

					current_missions.erase(id);
				}
				else
				{
					std::cout << u8"�̼� ���°� ������Ʈ�Ǿ����ϴ�: " << status << std::endl;
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