#ifndef __SPLASHTEXT_H__
#define __SPLASHTEXT_H__

#include "LocalEffect.h"

class SplashText : public LocalEffect {
private:
	std::string m_sText;
	uint32 m_uiColour;
	Renderer::FontType m_eFont;
	
public:
	SplashText(const std::string& id, const ShapeTemplate& shapedef, const TextureTemplate& texturedef);

	virtual const char* GetTypeName();
	static const char* TypeName();

	void SetText(std::string text, uint32 colour = 0xffffffff, Renderer::FontType font = Renderer::eFontTypeNormal);

protected:
	virtual void OnRender(Renderer& renderer, const FrameData& frame);
};

#endif
