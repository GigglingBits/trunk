#include "LevelMenu.h"
#include "FactoryManager.h"
#include "Debug.h"
#include "SoundEngine.h"

#define NO_BUTTON -1

LevelMenu::LevelMenu() : 
	Page(), 
	m_xPanelOptions(eButtonCommandIdOptions, s3eKeyFirst),
	m_xPanelSocial(eButtonCommandIdSocial, s3eKeyFirst) {
	m_pxBackground = FactoryManager::GetTextureFactory().Create("levelmenu_bg");

	memset(m_apxButtons, 0, sizeof(m_apxButtons));

	m_xPanelOptions.StateChanged.AddListener<LevelMenu>(this, &LevelMenu::ButtonPanelStateChangedEventHandler);
	m_xPanelSocial.StateChanged.AddListener<LevelMenu>(this, &LevelMenu::ButtonPanelStateChangedEventHandler);
}

LevelMenu::~LevelMenu() {
	m_xPanelOptions.StateChanged.RemoveListener<LevelMenu>(this, &LevelMenu::ButtonPanelStateChangedEventHandler);
	m_xPanelSocial.StateChanged.RemoveListener<LevelMenu>(this, &LevelMenu::ButtonPanelStateChangedEventHandler);

	if (m_pxBackground) {
		delete m_pxBackground;
	}

	for (int i = 0; i < LVLMENU_BTN_COUNT; i++) {
		if (m_apxButtons[i]) {
			delete m_apxButtons[i];
		}
	}

	SoundEngine::StopMusicFile();
}

Button* LevelMenu::GetButton(int col, int row) {
	return m_apxButtons[col + (row * LVLMENU_BTN_COLS)];
}

void LevelMenu::Initialize() {
	for (int i = 0; i < LVLMENU_BTN_COUNT; i++) {
		if (!m_apxButtons[i]) {
			m_apxButtons[i] = new Button(eButtonCommandIdOpenLevel, s3eKeyFirst, i);
		}

		char buf[8];
		snprintf(buf, sizeof(buf), "%i", i);
		m_apxButtons[i]->SetText(buf, 0xffffffff);
		m_apxButtons[i]->SetTexture(FactoryManager::GetTextureFactory().Create("button_level"));
	}

	m_xPanelOptions.Initialize();
	m_xPanelOptions.GetMainButton().SetTexture(FactoryManager::GetTextureFactory().Create("button_options"));

	m_xPanelSocial.Initialize();
	m_xPanelSocial.GetMainButton().SetTexture(FactoryManager::GetTextureFactory().Create("button_social"));

	SoundEngine::PlayMusicFileLoop("music/introsong.mp3");
}

void LevelMenu::OnUpdate(const FrameData& frame) {
	// update level buttons
	for (int i = 0; i < LVLMENU_BTN_COUNT; i++) {
		if (m_apxButtons[i]) {
			m_apxButtons[i]->Update(frame);
		}
	}

	// update other buttons
	if (m_xPanelSocial.IsPanelOpen() && m_xPanelOptions.IsEnabled()) {
		m_xPanelOptions.SetEnabled(false);
	} else if (!m_xPanelSocial.IsPanelOpen() && !m_xPanelOptions.IsEnabled()) {
		m_xPanelOptions.SetEnabled(true);
	}
	if (m_xPanelOptions.IsPanelOpen() && m_xPanelSocial.IsEnabled()) {
		m_xPanelSocial.SetEnabled(false);
	} else if (!m_xPanelOptions.IsPanelOpen() && !m_xPanelSocial.IsEnabled()) {
		m_xPanelSocial.SetEnabled(true);
	}
	m_xPanelOptions.Update(frame);
	m_xPanelSocial.Update(frame);

	if (m_pxBackground) {
		m_pxBackground->Update(frame.GetRealDurationMs());
    }
}

void LevelMenu::OnRender(Renderer& renderer, const FrameData& frame) {
	IW_CALLSTACK_SELF;

	renderer.SetViewport(m_xCamera.GetViewport());

    // background
    if (m_pxBackground) {
        VertexStreamScreen shape;        
        const CIwSVec2& screensize = frame.GetScreensize();
        shape.SetRect(CIwRect(0, 0, screensize.x, screensize.y));
        renderer.Draw(shape, *m_pxBackground);
    }
    
    // level buttons
	for (int i = 0; i < LVLMENU_BTN_COUNT; i++) {
		if (m_apxButtons[i]) {
			m_apxButtons[i]->Render(renderer, frame);
		}
	}

	// other buttons
	m_xPanelOptions.Render(renderer, frame);
	m_xPanelSocial.Render(renderer, frame);
}

void LevelMenu::OnDoLayout(const CIwSVec2& screensize) {
	int extents = GetScreenExtents();
	int margin = extents / 4;	// 25% 
		
	CIwRect menuarea(0, 0, screensize.x, screensize.y);
	menuarea.x += margin;
	menuarea.y += margin;
	menuarea.w -= 2*margin;
	menuarea.h -= 2*margin;

	int space = margin / 5;		// 25% / 5 = 5% 
	CIwRect buttonarea(0, 0, 
		((menuarea.w + space) / LVLMENU_BTN_COLS) - space,
		((menuarea.h + space) / LVLMENU_BTN_ROWS) - space);

	// level buttons
	for (int row = 0; row < LVLMENU_BTN_ROWS; row++) {
		for (int col = 0; col < LVLMENU_BTN_COLS; col++) {
			if (Button* p = GetButton(col, row)) {
				buttonarea.x = menuarea.x + col * (buttonarea.w + space);
				buttonarea.y = menuarea.y + row * (buttonarea.h + space);
				p->SetPosition(buttonarea);
			}
		}
	}

	// other buttons
	CIwRect button;
	button.h = extents / 7;	// height
	button.w = button.h;	// width

	button.x = (screensize.x - (3 * button.w) - (2 * space)) / 2;
	button.y = screensize.y * 10 / 13;
	m_xPanelOptions.GetMainButton().SetPosition(button);

	button.x += button.w + space;
	//m_xButtonOptions.SetPosition(button);

	button.x += button.w + space;
	m_xPanelSocial.GetMainButton().SetPosition(button);
}

void LevelMenu::ButtonPanelStateChangedEventHandler(const ButtonPanel& sender, const ButtonPanel::EventArgs& args) {
	ChangeState(args.IsPanelOpen, sender);
}

void LevelMenu::ChangeState(bool enable, const ButtonPanel& except) {
	for (int i = 0; i < LVLMENU_BTN_COUNT; i++) {
		if (m_apxButtons[i]) {
			m_apxButtons[i]->SetEnabled(enable);
		}
	}

	if (&except != &m_xPanelOptions) m_xPanelOptions.SetEnabled(enable);
	if (&except != &m_xPanelSocial) m_xPanelSocial.SetEnabled(enable);
}
