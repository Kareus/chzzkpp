#pragma once
#ifndef _CHZZK_CONFIG_
#define _CHZZK_CONFIG_

#ifndef _USE_CURL
#define _USE_CURL 1
#endif

namespace chzzkpp
{
	namespace config
	{
#ifdef __ANDROID__
		static char const OS_TYPE[] = "AOS";
#elif __APPLE__
		static char const OS_TYPE[] = "IOS";
#else
		static char const OS_TYPE[] = "PC";
#endif
	}
}

#endif