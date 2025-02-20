#ifndef __BUTTONCOMMANDID_H__
#define __BUTTONCOMMANDID_H__

enum ButtonCommandId {
	eButtonCommandIdNone			= 0,

	//// game ineraction
	//eButtonCommandIdUp			= 1,
	//eButtonCommandIdDown			= 2,
	//eButtonCommandIdLeft			= 3,
	//eButtonCommandIdRight			= 4,

	// application navigation
	eButtonCommandIdOpenTitleScreen,
	eButtonCommandIdOpenIntroMovie,
	eButtonCommandIdOpenWorldMenu,
	eButtonCommandIdOpenLevelMenu,
	
    eButtonCommandIdOpenWorld,
    eButtonCommandIdOpenLevel,
	eButtonCommandIdOpenNextLevel,
	eButtonCommandIdRestartLevel,
	eButtonCommandIdQuitLevel,

	// application interaction
	eButtonCommandIdSocial,
	eButtonCommandIdAchievements,
	eButtonCommandIdLeaderboards,
	eButtonCommandIdOptions,
	eButtonCommandIdSoundFx,
	eButtonCommandIdMusic,
	eButtonCommandIdHelp,
	eButtonCommandIdCredits,
	eButtonCommandIdToggleHud,
	eButtonCommandIdReload,
	eButtonCommandIdFacebook,
	
	// gnome game actions
	eButtonCommandIdSetHeroFocus,
};

#endif
