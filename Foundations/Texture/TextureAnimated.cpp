#include "TextureAnimated.h"
#include "Debug.h"

TextureAnimated::TextureAnimated(const TextureTemplate& texturedef) {
	Load(texturedef.GetSkeletonAnimation());
}

void TextureAnimated::Load(const std::string& skeleton) {
	m_xAnimation.Load(skeleton);
}

bool TextureAnimated::IsSkeleton() {
	return true;
}

BufferedAnimTexture* TextureAnimated::GetSkeleton() {
	return &m_xAnimation;
}

bool TextureAnimated::ContainsFrame(const std::string name) {
	return m_xAnimation.ConstainsAnimation(name);
}

bool TextureAnimated::SelectFrame(const std::string& id) {
	return m_xAnimation.SetAnimation(id);
}

bool TextureAnimated::ReSelectFrame(int health) {
	IW_CALLSTACK_SELF;
	IwError(("TextureAnimated::ReSelectFrame(int health) not implemented"));
	return true;
}

bool TextureAnimated::SelectFrame(const std::string& id, int health) {
	// ignore the health; it is not used in animations
	return SelectFrame(id);
}

std::string TextureAnimated::GetCurrentFrame() {
	return m_xAnimation.GetCurrentAnimationName();
}

void TextureAnimated::SetHorizontalFlip(bool flip) {
	m_xAnimation.SetFlipX(flip);
}

bool TextureAnimated::GetHorizontalFlip() {
	return m_xAnimation.GetFlipX();
}

void TextureAnimated::Update(uint16 timestep) {
	m_xAnimation.Update(timestep);
}
