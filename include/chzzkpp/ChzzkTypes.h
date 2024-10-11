#pragma once
#ifndef _CHZZK_TYPES_
#define _CHZZK_TYPES_

#include <vector>
#include <string>

namespace chzzkpp
{
	class invalid_status_exception : public std::exception
	{
		int code;
		std::string message;

	public:
		invalid_status_exception(int code, const std::string& message) : code(code), message(message)
		{

		}

		char const* what() const throw()
		{
			std::string str = "Status Code Error: " + std::to_string(code) + " " + message;
			return str.c_str();
		}
	};

	//parsed chzzk api object structures

	struct ChzzkUserData
	{
		bool hasProfile;							//whether the user has profile
		std::string userIDHash;						//user id hash
		std::string nickname;						//user nickname
		std::string profileImageUrl;				//user profile image url
		std::vector<std::string> penalties;			//unknown: penalties applied to the user
		bool officialNotiAgree;						//whether the user agreed on official notification
		std::string officialNotiAgreeUpdatedDate;	//official notification agree date
		bool verified;								//whether the user is verified
		bool loggedIn;								//whether the user is logged in
	};

	//info if the user is temporarily restricted
	struct ChzzkAccessTokenTemporaryRestrict
	{
		int createdTime;
		int duration;
		bool temporaryRestrict;
		int times;
	};

	//access token for chat
	struct ChzzkAccessToken
	{
		std::string accessToken;								//access token
		std::string extraToken;									//extra token
		bool realNameAuth;										//whether the user has real name authorized
		ChzzkAccessTokenTemporaryRestrict temporaryRestrict;	//temporal restrict info
	};

	struct ChzzkChannelFollowingInfo
	{
		bool available;			//whether following info is available. false when data missing
		bool following;			//whether the user follows the channel
		bool notification;		//whether the user activated the channel notification
		std::string followDate;	//date when the user started to follow the channel
	};

	//searched channel personal info available when logged in
	struct ChzzkChannelPersonalData
	{
		bool available;							//whether personal data is available. false when data missing
		ChzzkChannelFollowingInfo followInfo;	//following info
		bool privateUserBlock;					//whether the channel is blocked
	};

	struct ChzzkSubscriptionPaymentAvailability
	{
		bool iapAvailability;	//unknown: in-app payment availability
		bool iabAvailability;	//unknown: in-app billing availability
	};

	//simple channel info when searched
	struct ChzzkChannelInfo
	{
		std::string ID;							//channel ID
		std::string name;						//channel name
		std::string imageURL;					//url of the channel image
		bool verified;							//whether the channel is verified
		std::string description;				//channel description. could be null in some cases
		int followerCount;						//follower count. could be null in some cases
		ChzzkChannelPersonalData personalData;	//personal data (check ChzzkChannelPersonalData). could have some missing data in some cases
	};

	//channel detail
	struct ChzzkChannel : public ChzzkChannelInfo
	{
		std::string type;														//unknown: channel type. but I couldn't find any other value than "STREAMING"
		bool openLive;															//whether the channel is on live
		bool subscriptionAvailability;											//whether the channel subscription is available
		ChzzkSubscriptionPaymentAvailability subscriptionPaymentAvailability;	//subscription payment availability
		bool adMonetizationAvailability;										//whether ad monetization is available
		std::string userAdultStatus;											//adult status of user. "ADULT" when the user is adult
	};

	//live polling status.
	//actually, i don't know about these things
	struct ChzzkLivePollingStatus
	{
		std::string status;
		bool isPublishing;
		std::string playableStatus;
		int trafficThrottling;
		int callPeriodMilliSecond;
	};

	//live status
	struct ChzzkLiveStatus
	{
		std::string title;							//live title
		std::string status;							//live status. "OPEN" if live is on, "CLOSE" otherwise
		int concurrentUserCount;					//the number of current viewers
		int accumulatedUserCount;					//the number of accumulated viewers
		bool paidPromotion;							//whether the live is paid promotion
		bool adult;									//whether live is for adult
		bool krOnlyViewing;							//whether viewing is restricted in only korea. ex) 치지직 같이보기
		bool clipActive;							//whether clip is available
		std::string chatChannelID;					//unique id of the live chat
		std::vector<std::string> tags;				//live tags
		std::string categoryType;					//the first live category. ex) GAME, ETC. empty string if category is not set
		std::string liveCategory;					//the second live category, like the name of the game. empty string if category is not set
		std::string liveCategoryValue;				//output of the category name. ex) if liveCategory is "League_of_Legends", it will be "리그 오브 레전드"
		ChzzkLivePollingStatus livePollingStatus;	//live polling status
		std::string faultStatus;					//unknown: fault status
		std::string userAdultStatus;				//adult status of user. "ADULT" when the user is adult
		std::string blindType;						//unknown: blind type
		bool chatActive;							//whether chat is available
		std::string chatAvailableGroup;				//chat available group. ex) ALL, FOLLOWER, MANAGER
		std::string chatAvailableCondition;			//chat available condition. ex) only real name authenticated user can chat if the option is REAL_NAME, otherwise the option is NONE
		int minFollowerMinute;						//the minimum minutes on follow to chat
		bool chatDonationRankingExposure;			//whether the channel exposes donation ranking
	};

	//simple live data attached to other structs.
	struct ChzzkLiveBase
	{
		int ID;	//live id
		std::string title;						//live title
		std::string imageURL;					//url of the automatically-generated live image
		std::string defaultThumbnailImageURL;	//url of the default thumbnail image, which the live broadcaster set as
		int concurrentUserCount;				//the number of current viewers
		int accumulatedUserCount;				//the number of accumulated viewers
		std::string openDate;					//the date when the live started
		bool adult;								//whether live is for adult
		std::vector<std::string> tags;			//live tags
		std::string categoryType;				//the first live category. ex) GAME, ETC. empty string if category is not set
		std::string liveCategory;				//the second live category, like the name of the game. empty string if category is not set
		std::string liveCategoryValue;			//output of the category name. ex) if liveCategory is "League_of_Legends", it will be "리그 오브 레전드"
		std::string channelID;					//channel ID
		ChzzkChannelInfo channelInfo;			//channel info
		std::string blindType;					//unknown: blind type
	};

	//live struct when searched
	struct ChzzkLive : public ChzzkLiveBase
	{
		std::string chatChannelID;		//unique id of the live chat
		std::string livePlayback;		//JSON data of live playback data
		std::string dropsCampaignNo;	//TODO: new element to work
	};
	
	//live detail
	struct ChzzkLiveDetail : public ChzzkLive
	{
		std::string status;							//live status. "OPEN" if live is on, "CLOSE" otherwise
		std::string closeDate;						//the date when the live closed
		bool clipActive;							//whether clip is available
		bool chatActive;							//whether chat is available
		std::string chatAvailableGroup;				//chat available group. ex) ALL, FOLLOWER, MANAGER
		bool paidPromotion;							//whether the live is paid promotion
		std::string chatAvailableCondition;			//chat available condition. ex) only real name authenticated user can chat if the option is REAL_NAME, otherwise the option is NONE
		int minFollowerMinute;						//the minimum minutes on follow to chat
		std::vector<std::string> p2pQuality;		//qualities to use p2p (greed computing)
		ChzzkLivePollingStatus livePollingStatus;	//live polling status
		std::string userAdultStatus;				//adult status of user. "ADULT" when the user is adult
		bool chatDonationRankingExposure;			//whether the channel exposes donation ranking
		std::string adParameter;					//unknown: stringified json object of adParameter
	};

	//video struct when searched
	struct ChzzkVideoInfo
	{
		int videoNo;					//video number
		std::string ID;					//video ID
		std::string title;				//video title
		std::string type;				//video type ex) REPLAY, UPLOAD
		std::string publishDate;		//video published date. YYYY-MM-DD HH:MM:SS
		std::string thumbnailImageURL;	//url of thumbnail image
		int duration;					//video duration seconds
		int readCount;					//number of views
		int publishDateAt;				//publishDate as UNIX timestamp
		std::string channelID;			//channel id
		bool adult;						//whether video is for adult
		std::string categoryType;		//the first video category. ex) GAME, ETC. empty string if category is not set
		std::string videoCategory;		//the second videco category, like the name of the game. empty string if category is not set
		std::string videoCategoryValue;	//output of the category name. ex) if videoCategory is "League_of_Legends", it will be "리그 오브 레전드"
		std::string blindType;			//unknown: blind type
		ChzzkChannelInfo channelInfo;	//channel info
	};

	//video details
	struct ChzzkVideo : public ChzzkVideoInfo
	{
		std::string trailerURL;			//trailer url
		bool clipActive;				//whether clip is available
		bool exposure;					//unknown
		bool paidPromotion;				//whether video is paid promotion
		std::string inKey;				//unknown
		std::string liveOpenDate;		//open date if video is live replay
		std::string vodStatus;			//unknown
		std::string prevVideo;			//json data of previous video
		std::string nextVideo;			//json data of next video
		std::string userAdultStatus;	//adult status of user. "ADULT" when the user is adult
		std::string adParameter;		//unknown: stringified json object of adParameter
	};

	struct ChzzkResult
	{
		std::string keyword;	//search keyword
		int requested_size;		//requested search result size
		int offset;				//requested search result offset
	};

	struct ChzzkTopViewerResult : public ChzzkResult
	{
		std::vector<ChzzkLiveBase> lives;
	};

	struct ChzzkChannelResult : public ChzzkResult
	{
		std::vector<ChzzkChannelInfo> channels;
	};

	struct ChzzkLiveResult : public ChzzkResult
	{
		std::vector<ChzzkLive> lives;
	};

	struct ChzzkVideoResult : public ChzzkResult
	{
		std::vector<ChzzkVideoInfo> videos;
	};

	//TODO: below are different data from base structs...

	struct ChzzkRecommendChannelBase
	{
		std::string ID;					//channel ID
		std::string name;				//channel name
		std::string imageURL;			//url of the channel image
		bool verified;					//whether the channel is verified
		bool openLive;					//whether the channel is on live
		std::string title;				//live title
		int concurrentUserCount;		//the number of current viewers
		std::string liveCategoryValue;	//output of the category name. ex) if liveCategory is "League_of_Legends", it will be "리그 오브 레전드"
	};

	struct ChzzkRecommendChannel : public ChzzkRecommendChannelBase
	{
		std::string contentLineage;		//unknown: json object
	};

	struct ChzzkRecommendPartnerChannel : public ChzzkRecommendChannelBase
	{
		std::string originalName;	//original nickname
		bool isNewStreamer;			//whether the channel is new partner
	};

	struct ChzzkMissionInfo
	{
		std::string ID;						//mission donation id
		std::string missionText;			//mission text
		std::string channelID;				//channel ID of streamer
		std::string type;					//mission type. idk there is other types than ALONE
		int amount;							//pay amount
		int failCheeringRate;				//streamer receiving percent if mission failed
		std::string status;					//mission status
		bool success;						//whether mission is success
		int durationTime;					//mission duration time
		std::string startTime;				//time mission started
		std::string endTime;				//time mission ends
		std::string createdTime;			//time mission created

		std::string userIdHash;				//user id hash who created mission
		std::string userNickname;			//user nickname
		std::string userProfileImageURL;	//user profile image url
		bool userVerified;					//whether user is verified

		bool anonymous;						//whether user is anonymous

		//unknown-type channel;				//unknown: would be json object
		//unknown-type createdBadge;

		int totalAmount;					//total pay amount, including parcitipated missions
		std::string missionType;			//missionType: ALONE | GROUP | PARTICIPATION
		int participationCount;				//number of extra mission participants (ALONE if 0, GROUP otherwise)
		std::string relatedMissionID;		//ID of participating mission
	};

	struct ChzzkMissionResult
	{
		int page;	//mission page
		int size;	//max mission size of the page
		int totalCount;	//total mission count
		int totalPages;	//total page count

		std::vector<ChzzkMissionInfo> missions;
	};

	struct ChzzkDonationSetting
	{
		bool active;				//whether donation is active
		int minCurrencyPayAmount;	//minimum pay amount
	};

	struct ChzzkChatDonationSetting : public ChzzkDonationSetting
	{
		bool exposureDonationAmount;	//whether to show donated amount
	};

	struct ChzzkVideoDonationSetting : public ChzzkDonationSetting
	{
		int payAmountPerSecond;			//pay amount per video seconds
		int maxDurationLength;			//maxium length of video duration
		bool isYoutubeAllowed;			//whether the youtube video is allowed to donate
		bool isChzzkClipAllowed;		//whether the chzzk clip video is allowed to donate
		bool isAllowedForSubscribers;	//whether video donation is allowed for only subscribers
	};

	struct ChzzkMissionDonationSetting : public ChzzkDonationSetting
	{
		int maxCurrencyPayAmount;		//maximum pay amount (not set if the value is 0)
		int failCheeringRate;			//pay amount rate when mission failed
		int coolTime;					//mission cool time
	};
}
#endif