#include "LeaderboardsApple.h"

#include "IwDebug.h"
#include "Debug.h"


LeaderboardsApple::LeaderboardsApple() : m_bIsAuthenticating(false) {
	if (!IsAvailable()) {
		return;
	}

	Authenticate();
}

LeaderboardsApple::~LeaderboardsApple() {
}

const std::string& LeaderboardsApple::GetMediaName() {
	static const std::string s("Game Center");
	return s;
}

bool LeaderboardsApple::IsAvailable() {
	return s3eIOSGameCenterAvailable();
}

bool LeaderboardsApple::IsAuthenticated() {
	IW_CALLSTACK_SELF;

	return s3eIOSGameCenterGetInt(S3E_IOSGAMECENTER_LOCAL_PLAYER_IS_AUTHENTICATED) ? true : false;
}

bool LeaderboardsApple::WaitForAuthentication(uint16 milliseconds) {
	IW_CALLSTACK_SELF;

	// No need to authenticate; Authentication is done in constructor
	// Authenticate();
	
	const uint16 interval = 200;
	uint16 counter = 0;
	while (m_bIsAuthenticating) {
		s3eDeviceYield(interval);
		counter += interval;
		if (counter >= milliseconds) {
			return false;
		}
	}
	
	return true;
}

void LeaderboardsApple::SaveScore(const std::string& leaderboardid, unsigned long score) {
	IW_CALLSTACK_SELF;
	
	if (IsAuthenticated()) {
		s3eIOSGameCenterReportScore(score, leaderboardid.c_str(), SaveScoreCallback);
	}
}

bool LeaderboardsApple::ShowLeaderboard(const std::string& leaderboardid) {
	IW_CALLSTACK_SELF;

	if (!IsAuthenticated()) {
		IwError(("Game Center not authenticated. Cannot show leaderboards."));
		return false;
	}
	
	if (s3eIOSGameCenterLeaderboardShowGUI(leaderboardid.c_str(), S3E_IOSGAMECENTER_PLAYER_SCOPE_ALL_TIME) == S3E_RESULT_ERROR) {
		IwAssertMsg(LEADERBOARDS, false, ("Cannot open Game Center Leaderboards UI"));
		return false;
	}
	
	return true;
}

void LeaderboardsApple::SaveAchievement(const std::string& achievementid, uint8 percent) {
	IW_CALLSTACK_SELF;
	
	if (IsAuthenticated()) {
		s3eIOSGameCenterReportAchievement(achievementid.c_str(), percent, SaveAchievementCallback);
	}
}

bool LeaderboardsApple::ShowAchievements() {
	IW_CALLSTACK_SELF;

	if (!IsAuthenticated()) {
		IwError(("Game Center not authenticated. Cannot show achievements."));
		return false;
	}
	
	if (s3eIOSGameCenterAchievementsShowGUI() == S3E_RESULT_ERROR) {
		IwAssertMsg(LEADERBOARDS, false, ("Cannot open Game Center Achievements UI"));
		return false;
	}
	
	return true;
}

void LeaderboardsApple::Authenticate() {
	IW_CALLSTACK_SELF;

	if (!IsAuthenticated()) {
		s3eResult res = s3eIOSGameCenterAuthenticate(LeaderboardsApple::AuthenticationCallback, this);
		m_bIsAuthenticating = S3E_RESULT_SUCCESS == res;
		IwAssertMsg(LEADERBOARDS, m_bIsAuthenticating || IsAuthenticated(), ("Authentication should be in progress. But it seems not to be."));
	}
}

void LeaderboardsApple::AuthenticationCallback(s3eIOSGameCenterError* error, void* userData) {
	IW_CALLSTACK_SELF;
	
	LeaderboardsApple* ld = (LeaderboardsApple*)userData;
	if (ld) {
		IwAssertMsg(LEADERBOARDS, S3E_IOSGAMECENTER_ERR_NONE == *error, ("Game Center authentication failed: %s", ErrorAsString(*error)));
		ld->m_bIsAuthenticating = false;
	} else {
		IwAssertMsg(LEADERBOARDS, false, ("Invalid leaderboard handle!"));
	}
}

void LeaderboardsApple::SaveScoreCallback(s3eIOSGameCenterError* error) {
	IW_CALLSTACK_SELF;
	
	IwAssertMsg(LEADERBOARDS, S3E_IOSGAMECENTER_ERR_NONE == *error, ("Game Center score submission failed: %s", ErrorAsString(*error)));
}

void LeaderboardsApple::SaveAchievementCallback(s3eIOSGameCenterError* error) {
	IW_CALLSTACK_SELF;
	
	IwAssertMsg(LEADERBOARDS, S3E_IOSGAMECENTER_ERR_NONE == *error, ("Game Center achievement submission failed: %s", ErrorAsString(*error)));
}

const char* LeaderboardsApple::ErrorAsString(s3eIOSGameCenterError error) {
	switch(error) {
		case S3E_IOSGAMECENTER_ERR_NONE:
			return "NONE";
		case S3E_IOSGAMECENTER_ERR_PARAM:
			return "PARAM";
		case S3E_IOSGAMECENTER_ERR_UNAUTHENTICATED:
			return "UNAUTHENTICATED";
		case S3E_IOSGAMECENTER_ERR_FRIENDS_NOT_LOADED:
			return "FRIENDS_NOT_LOADED";
		case S3E_IOSGAMECENTER_ERR_ALREADY_REG:
			return "ALREADY_REG";
		case S3E_IOSGAMECENTER_ERR_GAME_UNRECOGNISED:
			return "GAME_UNRECOGNISED";
		case S3E_IOSGAMECENTER_ERR_AUTHENTICATION_IN_PROGRESS:
			return "AUTHENTICATION_IN_PROGRESS";
		case S3E_IOSGAMECENTER_ERR_INVALID_CREDENTIALS:
			return "INVALID_CREDENTIALS";
		case S3E_IOSGAMECENTER_ERR_UNDERAGE:
			return "UNDERAGE";
		case S3E_IOSGAMECENTER_ERR_COMMUNICATIONS_FAILURE:
			return "COMMUNICATIONS_FAILURE";
		case S3E_IOSGAMECENTER_ERR_CANCELLED:
			return "CANCELLED";
		case S3E_IOSGAMECENTER_ERR_USER_DENIED:
			return "DENIED";
		case S3E_IOSGAMECENTER_ERR_INVALID_PLAYER:
			return "INVALID_PLAYER";
		case S3E_IOSGAMECENTER_ERR_SCORE_NOT_SET:
			return "SCORE_NOT_SET";
		case S3E_IOSGAMECENTER_ERR_PARENTAL_CONTROLS_BLOCKED:
			return "PARENTAL_CONTROLS_BLOCKED";
		case S3E_IOSGAMECENTER_ERR_INVALID_MATCH_REQUEST:
			return "INVALID_MATCH_REQUEST";
		case S3E_IOSGAMECENTER_ERR_DEVICE:
		default:
			return "DEVICE/UNKNOWN";
	}
}

