#ifndef __PAGEMANAGER_H__
#define __PAGEMANAGER_H__

#include "Page.h"
#include "PageSettings.h"
#include "Curtain.h"

class PageManager : public Renderable {
private:
	Page* m_pxNextPage;
	Page* m_pxCurrentPage;

    PageSettings m_xPageSettings;
	bool m_bRestartRequired;

	Curtain m_xCurtain;

public:
	PageManager();
	~PageManager();

    void SetLevel(int level);

    
    void StartTitleScreen();
    void StartIntroMovie();
    void StartWorldMenu();
	void StartLevelMenu();
	void StartLevel();
    void StartNextLevel();
	
private:
	Page* CreateDefaultPage();
	Page* CreateNextPage(Page* oldpage);

	void SetNextPage(Page* page);

	void ApplyNextPage();
	
	std::string GetCurrentLevelName();
	std::string GetNextLevelName();
	
	void SubmitAnalytics(Page* oldpage, Page* newpage);
	
protected:
	virtual void OnUpdate(const FrameData& frame);
	virtual void OnRender(Renderer& renderer, const FrameData& frame);
};

#endif
