#pragma once
#ifndef _CHZZK_PATH_
#define _CHZZK_PATH_

#include <string>

namespace chzzkpp
{
	static char const CHZZK_API_PATH_PREFIX_SEARCH_CHANNEL[] = "https://api.chzzk.naver.com/service/v1/search/channels";
	static char const CHZZK_API_PATH_PREFIX_SEARCH_LIVE[] = "https://api.chzzk.naver.com/service/v1/search/lives";
	static char const CHZZK_API_PATH_PREFIX_SEARCH_VIDEO[] = "https://api.chzzk.naver.com/service/v1/search/videos";
	static char const CHZZK_API_PATH_PREFIX_LIVE_DETAIL[] = "https://api.chzzk.naver.com/service/v2/channels/";
	static char const CHZZK_API_PATH_PREFIX_LIVE_STATUS[] = "https://api.chzzk.naver.com/polling/v2/channels/";
	static char const CHZZK_API_PATH_PREFIX_TOP_VIEWER_LIVES[] = "https://api.chzzk.naver.com/service/v1/lives";
	static char const CHZZK_API_PATH_PREFIX_CHANNEL[] = "https://api.chzzk.naver.com/service/v1/channels/";
	static char const CHZZK_API_PATH_PREFIX_VIDEO[] = "https://api.chzzk.naver.com/service/v2/videos/";
	static char const CHZZK_API_PATH_PREFIX_ACCESS_TOKEN[] = "https://comm-api.game.naver.com/nng_main/v1/chats/access-token";

	static char const CHZZK_API_PATH_USER[] = "https://comm-api.game.naver.com/nng_main/v1/user/getUserStatus";
	static char const CHZZK_API_PATH_RECOMMENDATION_LIVES[] = "https://api.chzzk.naver.com/service/v1/home/recommended";
	static char const CHZZK_API_PATH_RECOMMENDATION_CHANNELS[] = "https://api.chzzk.naver.com/service/v1/home/recommendation-channels";
	static char const CHZZK_API_PATH_RECOMMENDATION_PARTNERS[] = "https://api.chzzk.naver.com/service/v1/streamer-partners/recommended";

	static char const CHZZK_API_PATH_SUFFIX_LIVE_STATUS[] = "/live-status";
	static char const CHZZK_API_PATH_SUFFIX_LIVE_DETAIL[] = "/live-detail";
	static char const CHZZK_API_PATH_SUFFIX_MISSIONS[] = "/donations/missions";
	static char const CHZZK_API_PATH_SUFFIX_DONATION_CHAT_SETTING[] = "/donations/chat-setting";
	static char const CHZZK_API_PATH_SUFFIX_DONATION_VIDEO_SETTING[] = "/donations/video-setting";
	static char const CHZZK_API_PATH_SUFFIX_DONATION_MISSION_SETTING[] = "/donations/mission-setting";

	static char const CHZZK_CHAT_WEBSOCKET_PATH_PREFIX[] = "wss://kr-ss";
	static char const CHZZK_CHAT_WEBSOCKET_PATH_SUFFIX[] = ".chat.naver.com/chat";
}
#endif