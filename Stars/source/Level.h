#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "Page.h"
#include "Body.h"
#include "AppPanel.h"
#include "GameFoundation.h"
#include "LevelBackground.h"
#include "LevelHud.h"
#include "Star.h"
#include "Camera.h"
#include "LevelInteractor.h"

#define LEVEL_COMPLETION_DELAY 10000

class Level : public Page {
private:
	CIwFVec2 m_xWorldSize;

	Camera m_xCamera;
	LevelBackground m_xBackground;

	GameFoundation m_xGame;

	bool m_bIsPaused;
	
	AppPanel m_xAppPanel;
	LevelHud m_xHud;

	LevelInteractor m_xInteractor;

	int m_iCompletionTimer;

public:
	Level(const CIwFVec2& worldsize, float dustrequirement, std::string background);
	virtual ~Level();

	virtual void Initialize();

	void Add(Body* body);

	bool GetCompletionInfo(GameFoundation::CompletionInfo& info);

	GameFoundation& GetGameFoundation();

	virtual const std::string& GetResourceGroupName();
	
protected:
	virtual void OnUpdate(const FrameData& frame);
	virtual void OnRender(Renderer& renderer, const FrameData& frame);
	virtual void OnDoLayout(const CIwSVec2& screensize);

private:
	void CreateStar();

	void CreateHud();
	void DestroyHud();

	float GetStarMoveForce();
	float GetStarRestForce();
	CIwFVec2 GetStarStartPosition();

	void SetPaused(bool paused);
	bool IsPaused();
	static int32 AppPausedCallback(void* systemData, void* userData);

	CIwFVec2 CalculateRelativeSoundPosition(const CIwFVec2& worldpos);
	
private:
	void AppPanelStateChangedEventHandler(const ButtonPanel& sender, const ButtonPanel::EventArgs& args);

	void BeginDrawPathEventHandler(const LevelInteractor& sender, const CIwFVec2& pos);
	void EndDrawPathHandler(const LevelInteractor& sender, const LevelInteractor::PathEventArgs& path);

	void QuakeImpactEventHandler(const GameFoundation& sender, const GameFoundation::QuakeImpactArgs& args);
};

#endif
