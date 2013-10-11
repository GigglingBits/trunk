#include "s3e.h"

#include "Configuration.h"

Configuration* Configuration::s_pxInstance = NULL;

Configuration::Configuration() {
	char tmpstr[S3E_CONFIG_STRING_MAX] = {'\0'};
	int tmpint = 0;
	if (S3E_RESULT_SUCCESS == s3eConfigGetInt("Game", "showstats", &tmpint)) {
		ShowStats = tmpint == 1;
	} else {
		ShowStats = 0;
	}

	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "bodies", tmpstr)) {
		BodiesFile = tmpstr;
	} else {
		BodiesFile = "bodies.xml";
	}

	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "levels", tmpstr)) {
		LevelsFile = tmpstr;
	} else {
		LevelsFile = "levels.xml";
	}

	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "introsong", tmpstr)) {
		IntroSong = tmpstr;
	} else {
		IntroSong = "music/introsong.mp3";
	}

	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "levelsong", tmpstr)) {
		LevelSong = tmpstr;
	} else {
		LevelSong = "music/levelsong.mp3";
	}

	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "lostsong", tmpstr)) {
		LostSong = tmpstr;
	} else {
		LostSong = "music/lostsong.mp3";
	}

	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "wonsong", tmpstr)) {
		WonSong = tmpstr;
	} else {
		WonSong = "music/wonsong.mp3";
	}

	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "httpbodies", tmpstr)) {
		HttpBodiesFile = tmpstr;
	} else {
		HttpBodiesFile = "";
	}

	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "httplevels", tmpstr)) {
		HttpLevelsFile = tmpstr;
	} else {
		HttpLevelsFile = "";
	}

	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "maxvisibleworldsize", tmpstr)) {
		MaxVisibleWorldSize = atof(tmpstr);
	} else {
		MaxVisibleWorldSize = 20.0f;
	}
	
	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "worldmargin", tmpstr)) {
		WorldMargin = atof(tmpstr);
	} else {
		WorldMargin = 5.0f;
	}
	
	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "pathspeed", tmpstr)) {
		PathSpeed = atof(tmpstr);
	} else {
		PathSpeed = 2.0f;
	}
	
	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "objectspeed", tmpstr)) {
		ObjectSpeed = atof(tmpstr);
	} else {
		ObjectSpeed = 2.0f;
	}
	
	if (S3E_RESULT_SUCCESS == s3eConfigGetString("Game", "pathmaxlength", tmpstr)) {
		PathMaxLength = atof(tmpstr);
	} else {
		PathMaxLength = 10.0f;
	}
}

void Configuration::Initialize() {
	GetInstance();
}

void Configuration::Terminate() {
	if (s_pxInstance) {
		delete s_pxInstance;
		s_pxInstance = NULL;
	}
}

Configuration& Configuration::GetInstance() {
	if (!s_pxInstance) {
		s_pxInstance = new Configuration();
	}
	return *s_pxInstance;
}

