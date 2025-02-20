#ifndef __APPANALYTICS_H__
#define __APPANALYTICS_H__

#include "Analytics.h"

class AppAnalytics {
public:
	void RegisterTitleScreenOpened();
	void RegisterWorldMenuOpened();
	void RegisterLevelMenuOpened();

	void RegisterLevelStarted(const std::string& id);
	void RegisterLevelCompleted(const std::string& id, int score, int stars, bool achieved);
	void RegisterLevelCancelled(const std::string& id);

	void RegisterFacebookOpened();
	void RegisterLeaderboardsOpened();
};

#endif
