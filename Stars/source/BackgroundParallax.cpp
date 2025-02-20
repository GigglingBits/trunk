#include "BackgroundParallax.h"
#include "FactoryManager.h"
#include "Debug.h"
#include "DeviceInfo.h"

BackgroundParallax::BackgroundParallax() : m_pxBackground(NULL), m_pxBackgroundFar(NULL), m_fParallaxCorrection(0.0f), m_xBackgroundRect(0, 0, 0, 0) {
	SetRenderingLayer(Renderer::eRenderingLayerBackground);
}

BackgroundParallax::~BackgroundParallax() {
	if (m_pxBackground) {
		delete m_pxBackground;
	}
	if (m_pxBackgroundFar) {
		delete m_pxBackgroundFar;
	}
}

void BackgroundParallax::Initialize() {
	m_pxBackground = FactoryManager::GetTextureFactory().Create("background_stars");
	m_pxBackgroundFar = FactoryManager::GetTextureFactory().Create("background_stars_far");
}

void BackgroundParallax::OnDoLayout(const CIwSVec2& screensize) {
	int extents = GetScreenExtents();

	int margin = extents / 10;
	m_xBackgroundRect = CIwRect(
		-margin,
		-margin,
		screensize.x + (2 * margin),
		screensize.y + (2 * margin));

	int dpi = DeviceInfo::GetInstance().GetScreenDpi();
	m_fParallaxCorrection = dpi * 0.004f * (extents * 0.2f / dpi);
}

void BackgroundParallax::OnUpdate(const FrameData& frame) {
	if (m_pxBackground) {
		m_pxBackground->Update(frame.GetRealDurationMs());
    }
	if (m_pxBackgroundFar) {
		m_pxBackgroundFar->Update(frame.GetRealDurationMs());
    }
}

void BackgroundParallax::OnRender(Renderer& renderer, const FrameData& frame) {
	const LocationServices::DeviceOrientation& orientation =
		LocationServices::GetInstance().GetDeviceOrientation();

	if (m_pxBackgroundFar) {
		CIwSVec2 orientationoffset(
			orientation.x * -m_fParallaxCorrection * 2.0f,
			orientation.y * m_fParallaxCorrection * 2.0f);
		RenderBackground(renderer, *m_pxBackgroundFar, orientationoffset);
	}
	if (m_pxBackground) {
		CIwSVec2 orientationoffset(
			orientation.x * -m_fParallaxCorrection,
			orientation.y * m_fParallaxCorrection);
		RenderBackground(renderer, *m_pxBackground, orientationoffset);
	}
}

void BackgroundParallax::RenderBackground(Renderer& renderer, Texture& texture, const CIwSVec2& orientationoffset) {
	CIwRect rect(m_xBackgroundRect);
	rect.x += orientationoffset.x;
	rect.y += orientationoffset.y;
	
	VertexStreamScreen shape;
	shape.SetRect(rect);
	
	renderer.Draw(shape, texture);
}

