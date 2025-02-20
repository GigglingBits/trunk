#ifndef __TITLEMENU_H__
#define __TITLEMENU_H__

#include "Page.h"
#include "Button.h"
#include "SocialPanel.h"
#include "OptionsPanel.h"
#include "IwList.h"
#include "Body.h"
#include "Camera.h"

class TitleMenu : public Page {
private:
	Camera m_xCamera;

	Texture* m_pxBackground;
	
	Button m_xButtonPlay;
	SocialPanel m_xPanelSocial;
	OptionsPanel m_xPanelOptions;

public:
	TitleMenu();
	virtual ~TitleMenu();

	virtual void Initialize();

protected:
	virtual void OnUpdate(const FrameData& frame);
	virtual void OnRender(Renderer& renderer, const FrameData& frame);
	virtual void OnDoLayout(const CIwSVec2& screensize);

private:
	void ChangeState(bool enable, const ButtonPanel& except);
	void ButtonPanelStateChangedEventHandler(const ButtonPanel& sender, const ButtonPanel::EventArgs& args);
};

#endif
