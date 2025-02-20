#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__

#include "Page.h"
#include "BackgroundParallax.h"
#include "Button.h"
#include "IwList.h"
#include "Body.h"
#include "Camera.h"
#include "InputManager.h"

#include "AnimTexture.h"

class TitleScreen : public Page {
private:
	Camera m_xCamera;

	BackgroundParallax m_xBackground;
	Button m_xButtonTitle;
	Button m_xButtonFacebook;
	Button m_xButtonLeaderboards;
	Button m_xButtonMovie;

	bool m_bHasFacebookButton;
	bool m_bHasLeaderboardsButton;
	bool m_bHasMovieButton;
	
	InputManager::VirtualButton m_xBackButton;

public:
	TitleScreen();
	virtual ~TitleScreen();

	virtual void Initialize();
	
protected:
	virtual void OnDoLayout(const CIwSVec2& screensize);
	virtual void OnUpdate(const FrameData& frame);
	virtual void OnRender(Renderer& renderer, const FrameData& frame);

private:
	void OpenFacebook();
	void OpenLeaderboards();
	
	void ButtonPressedEventHandler(const Button& sender, const Button::EventArgs& args);
	void ButtonReleasedEventHandler(const InputManager::VirtualButton& sender, const InputManager::VirtualButton::EventArgs& args);
};

#endif
