#include <sstream>

#include "Debug.h"
#include "Renderer.h"
#include "IwGxFont.h"

Renderer::Renderer() {
	m_xScreenOffset = CIwSVec2::g_Zero;
	m_eCurrentRenderingLayer = eRenderingLayerGameDynamicObjects;

	ResetAmbientLightColour();
	
	memset(m_apxFonts, 0, sizeof(m_apxFonts));
}

void Renderer::SetAmbientLightColour(uint32 colour) {
	m_uiAmbientLight = colour;
}

void Renderer::ResetAmbientLightColour() {
	m_uiAmbientLight = RENDERER_WHITE_LIGHT;
}

void Renderer::SetFonts(const std::string& large, const std::string& normal, const std::string& small, const std::string& system) {

	CIwGxFont* font;
	font = (CIwGxFont*)IwGetResManager()->GetResNamed(large.c_str(), "CIwGxFont");
	m_apxFonts[eFontTypeLarge] = font;

	font = (CIwGxFont*)IwGetResManager()->GetResNamed(normal.c_str(), "CIwGxFont");
	m_apxFonts[eFontTypeNormal] = font;

	font = (CIwGxFont*)IwGetResManager()->GetResNamed(small.c_str(), "CIwGxFont");
	m_apxFonts[eFontTypeSmall] = font;

	font = (CIwGxFont*)IwGetResManager()->GetResNamed(system.c_str(), "CIwGxFont");
	m_apxFonts[eFontTypeSystem] = font;

	IwGxFontSetFont(font);
}

CIwMaterial* Renderer::CreateGxCacheMaterial(CIwTexture* texture, bool additivebelnding) {
	CIwMaterial* mat = IW_GX_ALLOC_MATERIAL();
	mat->SetColAmbient(m_uiAmbientLight);
	mat->SetModulateMode(CIwMaterial::MODULATE_RGB);
	mat->SetAlphaMode(additivebelnding ? CIwMaterial::ALPHA_ADD : CIwMaterial::ALPHA_BLEND);
	mat->SetTexture(texture);
	return mat;
}

CIwMaterial* Renderer::CreateGxCacheMaterial(uint colour) {
	CIwMaterial* mat = IW_GX_ALLOC_MATERIAL();
	mat->SetColAmbient(colour);
	return mat;
}

CIwFVec2 Renderer::GetScreenCenterWorldSpace() {
	// where in the world is the screen center?
	CIwSVec2 screencenter = m_xViewport.GetViewportSize();
	screencenter.x /= 2;	
	screencenter.y /= 2; 

	// how far away is the world center from the screen center?
	return m_xViewport.ViewToWorld(screencenter);
}

void Renderer::SetViewport(const Viewport& viewport) {
	m_xViewport = viewport;
	m_xScreenOffset = m_xViewport.GetScreenViewOffset();
}

void Renderer::SetDefaultRederingLayer() {
	SetRenderingLayer(eRenderingLayerGameDynamicObjects);
}

void Renderer::SetRenderingLayer(RenderingLayer renderinglayer) {
	m_eCurrentRenderingLayer = renderinglayer;
}

void Renderer::Draw(VertexStreamWorld& shape, Texture& texture) {
	DrawTexture<VertexStreamWorld, CIwFVec2>(shape, texture);
}

void Renderer::Draw(VertexStreamScreen& shape, Texture& texture) {
	DrawTexture<VertexStreamScreen, CIwSVec2>(shape, texture);
}

template <class TVertexStream, class TVertex>
void Renderer::DrawTexture(TVertexStream& shape, Texture& texture) {
	IW_CALLSTACK_SELF;

	if (shape.GetVertCount() <= 2) {
		// IwAssertMsg(MYAPP, false, ("Shape is not a valid polygon"));
		return;
	}

	if (texture.IsImage()) {
		DrawImage(
			texture.GetImage(), 
			shape.GetVerts(), 
			shape.GetVertCount(), 
			texture.GetHorizontalFlip());
	} else if (texture.IsPattern()) {
		DrawPolygon(
			shape.GetVerts(), 
			shape.GetVertCount(), 
			texture.GetPattern());
	} else if (texture.IsSkeleton()) {
		if (BufferedAnimTexture* anim = texture.GetSkeleton()) {
			anim->SetAll(shape.GetVerts(), shape.GetVertCount());
			DrawImage(anim->GetBufferedSpriteSheet(),
					  anim->GetBufferedXYs(TVertex()),
					  anim->GetBufferedUVs(),
					  anim->GetBufferedCols(),
					  anim->GetBufferedVertexCount());
		} else {
			IwAssertMsg(MYAPP, false, ("No skeleton available for rendering!"));
		}
	} else if (texture.IsColour()) {
		uint32 col = texture.GetColour();
		TVertex* verts = shape.GetVerts();
		int count = shape.GetVertCount();

		// closed or open poly?
		bool isclosed = (verts[0] == verts[count - 1]);
		DrawPolygon(
			verts, 
			count,
			isclosed ? 0x00000000 : col,
			isclosed ? col : 0x00000000);
	} else {
		IwAssertMsg(MYAPP, false, ("No rendering mechanism implemented for current texture frame!"));
	}
}

void Renderer::DrawRect(const CIwRect& rect, uint32 framecol, uint32 bodycol) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&CIwSVec2::g_Zero);
	const int count = 5;

	CIwSVec2 vertices[count];
	vertices[0].x = rect.x, vertices[0].y = rect.y;
	vertices[1].x = rect.x, vertices[1].y = rect.y + rect.h;
	vertices[2].x = rect.x + rect.w, vertices[2].y = rect.y + rect.h;
	vertices[3].x = rect.x + rect.w, vertices[3].y = rect.y;
	vertices[4].x = rect.x, vertices[4].y = rect.y;

	CIwFVec2* polystream = CreatGxCacheVertexStream(vertices, count);
	DrawPolygonSubPixel(polystream, count, framecol, bodycol);
}

void Renderer::DrawImage(CIwTexture* image, const CIwFVec2& pos, const CIwFVec2& size, bool flipped) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&m_xScreenOffset);
	const int count = 4;
	
	CIwFVec2 vertices[count];
	vertices[0].x = pos.x, vertices[0].y = pos.y;
	vertices[1].x = pos.x + size.x, vertices[1].y = pos.y;
	vertices[2].x = pos.x + size.x, vertices[2].y = pos.y + size.y;
	vertices[3].x = pos.x, vertices[3].y = pos.y + size.y;

	CIwFVec2* polystream = CreatGxCacheVertexStream((CIwFVec2*)vertices, count);
	DrawPolygonSubPixel(polystream, count, image, flipped);
}

void Renderer::DrawImage(CIwTexture* image, CIwFVec2 vertices[], int count, bool flipped) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&m_xScreenOffset);

	CIwFVec2* polystream = CreatGxCacheVertexStream(vertices, count);
	DrawPolygonSubPixel(polystream, count, image, flipped);
}

void Renderer::DrawImage(CIwTexture* image, CIwSVec2 vertices[], int count, bool flipped) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&CIwSVec2::g_Zero);

	CIwFVec2* polystream = CreatGxCacheVertexStream(vertices, count);
	DrawPolygonSubPixel(polystream, count, image, flipped);
}

void Renderer::DrawImage(CIwTexture* image, CIwFVec2 vertices[], CIwFVec2 uvs[], uint32 cols[], int count, bool additiveblend) {
	IW_CALLSTACK_SELF;
	
	IwGxSetScreenSpaceOrg(&m_xScreenOffset);
	
	CIwFVec2* gxverts = CreatGxCacheVertexStream(vertices, count);
	CIwMaterial* gxmat = CreateGxCacheMaterial(image, additiveblend);
	DrawImageSubPixel(gxmat, gxverts, CreatGxCacheUvStream(uvs, count), CreatGxCacheColourStream(cols, count), count);
}

void Renderer::DrawImage(CIwTexture* image, CIwSVec2 vertices[], CIwFVec2 uvs[], uint32 cols[], int count, bool additiveblend) {
	IW_CALLSTACK_SELF;
	
	IwGxSetScreenSpaceOrg(&CIwSVec2::g_Zero);
	
	CIwFVec2* gxverts = CreatGxCacheVertexStream(vertices, count);
	CIwMaterial* gxmat = CreateGxCacheMaterial(image, additiveblend);
	DrawImageSubPixel(gxmat, gxverts, CreatGxCacheUvStream(uvs, count), CreatGxCacheColourStream(cols, count), count);
}

void Renderer::DrawPolygon(CIwSVec2 vertices[], int count, CIwTexture* image) {
	DrawPolygon<CIwSVec2>(vertices, count, image);
}

void Renderer::DrawPolygon(CIwFVec2 vertices[], int count, CIwTexture* image) {
	DrawPolygon<CIwFVec2>(vertices, count, image);
}

template <class TVertex>
void Renderer::DrawPolygon(TVertex vertices[], int count, CIwTexture* image) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&m_xScreenOffset);

	CIwFVec2* polystream = CreatGxCacheVertexStream(vertices, count);
	CIwFVec2* wrapuv = CreatGxCacheTiledUvStream(polystream, count, image);
	IwGxSetUVStream(wrapuv);

	IwGxSetColStream(NULL);

	IwGxSetMaterial(CreateGxCacheMaterial(image));

	IwGxSetVertStreamScreenSpace(polystream, count);

	int32 slot = IwGxGetScreenSpaceSlot();
	IwGxSetScreenSpaceSlot((int32)m_eCurrentRenderingLayer);
	IwGxDrawPrims(IW_GX_TRI_FAN, NULL, count);
	IwGxSetScreenSpaceSlot(slot);
}

void Renderer::DrawPolygonSubPixel(CIwFVec2 vertices[], int count, CIwTexture* image, bool flipped) {
	IW_CALLSTACK_SELF;

	// standard orientation
	static const CIwFVec2 standarduv[4] = {
		CIwFVec2(0.0f, 1.0f),
		CIwFVec2(1.0f, 1.0f),
		CIwFVec2(1.0f, 0.0f),
		CIwFVec2(0.0f, 0.0f),
	};

	// horizontally flipped orientation
	static const CIwFVec2 flippeduv[4] = {
		CIwFVec2(1.0f, 1.0f),
		CIwFVec2(0.0f, 1.0f),
		CIwFVec2(0.0f, 0.0f),
		CIwFVec2(1.0f, 0.0f),
	};

	DrawImageSubPixel(CreateGxCacheMaterial(image), vertices, (CIwFVec2*)(flipped ? flippeduv : standarduv), NULL, count);
}

void Renderer::DrawImageSubPixel(CIwMaterial* image, CIwFVec2 vertices[], CIwFVec2 uvs[], CIwColour cols[], int count) {
	IW_CALLSTACK_SELF;
	
	IwGxSetMaterial(image);
	IwGxSetUVStream(uvs);
	IwGxSetColStream(cols, count);
	IwGxSetVertStreamScreenSpace(vertices, count);
	
	int32 slot = IwGxGetScreenSpaceSlot();
	IwGxSetScreenSpaceSlot((int32)m_eCurrentRenderingLayer);
	IwGxDrawPrims(IW_GX_QUAD_LIST, NULL, count);
	IwGxSetScreenSpaceSlot(slot);
}

void Renderer::DrawPolygon(CIwFVec2 vertices[], int count, uint32 framecol, uint32 bodycol) {
	IW_CALLSTACK_SELF;

	if (!vertices || count == 0) {
		IwAssertMsg(MYAPP, false, ("Cannot render empty polygon"));
		return;
	}

	IwGxSetScreenSpaceOrg(&m_xScreenOffset);

	CIwFVec2* polystream = CreatGxCacheVertexStream(vertices, count);
	DrawPolygonSubPixel(polystream, count, framecol, bodycol);
}

void Renderer::DrawPolygon(CIwFVec2 vertices[], int count, uint32 framecols[], uint32 bodycols[]) {
	IW_CALLSTACK_SELF;

	if (!vertices || count == 0) {
		IwAssertMsg(MYAPP, false, ("Cannot render empty polygon"));
		return;
	}

	IwGxSetScreenSpaceOrg(&m_xScreenOffset);

	CIwFVec2* polystream = CreatGxCacheVertexStream(vertices, count);
	CIwColour* framecolstream = framecols ? CreatGxCacheColourStream(framecols, count) : NULL;
	CIwColour* bodycolstream = bodycols ? CreatGxCacheColourStream(bodycols, count) : NULL;
	DrawPolygonSubPixel(polystream, count, framecolstream, bodycolstream);
}

void Renderer::DrawPolygon(CIwSVec2 vertices[], int count, uint32 framecol, uint32 bodycol) {
	IW_CALLSTACK_SELF;

	if (!vertices || count == 0) {
		IwAssertMsg(MYAPP, false, ("Cannot render empty polygon"));
		return;
	}

	IwGxSetScreenSpaceOrg(&CIwSVec2::g_Zero);

	CIwFVec2* polystream = CreatGxCacheVertexStream(vertices, count);
	DrawPolygonSubPixel(polystream, count, framecol, bodycol);
}

void Renderer::DrawPolygon(CIwSVec2 vertices[], int count, uint32 framecols[], uint32 bodycols[]) {
	IW_CALLSTACK_SELF;

	if (!vertices || count == 0) {
		IwAssertMsg(MYAPP, false, ("Cannot render empty polygon"));
		return;
	}

	IwGxSetScreenSpaceOrg(&CIwSVec2::g_Zero);

	CIwFVec2* polystream = CreatGxCacheVertexStream(vertices, count);
	CIwColour* framecolstream = framecols ? CreatGxCacheColourStream(framecols, count) : NULL;
	CIwColour* bodycolstream = bodycols ? CreatGxCacheColourStream(bodycols, count) : NULL;
	DrawPolygonSubPixel(polystream, count, framecolstream, bodycolstream);
}

void Renderer::DrawPolygonSubPixel(CIwFVec2 vertices[], int count, uint32 framecol, uint32 bodycol) {
	IW_CALLSTACK_SELF;

	if (!(bodycol || framecol)) {
		return;
	}

	IwGxSetUVStream(NULL);
	IwGxSetColStream(NULL);

	// set stream once, use it for 2 primitive drawing instructions
	IwGxSetVertStreamScreenSpace(vertices, count);

	int32 slot = IwGxGetScreenSpaceSlot();
	IwGxSetScreenSpaceSlot((int32)m_eCurrentRenderingLayer);
	if (bodycol) {
	    IwGxSetMaterial(CreateGxCacheMaterial(bodycol));
		IwGxDrawPrims(IW_GX_TRI_FAN, NULL, count);
	}
	if (framecol) {
	    IwGxSetMaterial(CreateGxCacheMaterial(framecol));
		IwGxDrawPrims(IW_GX_LINE_STRIP, NULL, count);
	}
	IwGxSetScreenSpaceSlot(slot);
}

void Renderer::DrawPolygonSubPixel(CIwFVec2 vertices[], int count, CIwColour framecols[], CIwColour bodycols[]) {
	IW_CALLSTACK_SELF;

	IwGxSetUVStream(NULL);

	IwGxSetVertStreamScreenSpace(vertices, count);

	int32 slot = IwGxGetScreenSpaceSlot();
	IwGxSetScreenSpaceSlot((int32)m_eCurrentRenderingLayer);
	IwGxSetMaterial(CreateGxCacheMaterial(0xffffffff));
	
	if (bodycols) {
		IwGxSetColStream(bodycols, count);
		IwGxDrawPrims(IW_GX_TRI_FAN, NULL, count);
	}
	if (framecols) {
		IwGxSetColStream(framecols, count);
		IwGxDrawPrims(IW_GX_LINE_STRIP, NULL, count);
	}

	IwGxSetScreenSpaceSlot(slot);
}

void Renderer::DrawLine(const CIwFVec2& startpos, const CIwFVec2& endpos, uint32 col) {
	IW_CALLSTACK_SELF;

	CIwFVec2 vertices[2] = {startpos, endpos};
	DrawPolygon(vertices, 2, col, 0);
}

void Renderer::DrawLine(CIwFVec2 vertices[], int count, uint32 vertexcols[]) {
	IW_CALLSTACK_SELF;

	DrawPolygon(vertices, count, vertexcols, NULL);
}

void Renderer::DrawLine(const CIwSVec2& startpos, const CIwSVec2& endpos, uint32 col) {
	IW_CALLSTACK_SELF;

	CIwSVec2 vertices[2] = {startpos, endpos};
	DrawPolygon(vertices, 2, col, 0);
}

void Renderer::DrawImage(CIwTexture* image, const CIwSVec2& pos, const CIwSVec2& size, bool flipped) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&CIwSVec2::g_Zero);
	const int count = 4;
	
	CIwSVec2 vertices[count];
	vertices[0].x = pos.x, vertices[0].y = pos.y + size.y;
	vertices[1].x = pos.x + size.x, vertices[1].y = pos.y + size.y;
	vertices[2].x = pos.x + size.x, vertices[2].y = pos.y;
	vertices[3].x = pos.x, vertices[3].y = pos.y;

	CIwFVec2* polystream = CreatGxCacheVertexStream((CIwSVec2*)vertices, count);
	DrawPolygonSubPixel(polystream, count, image, flipped);
}

void Renderer::DrawText(std::string text, const CIwRect& rect, FontType fonttype, uint32 col, TextAlignmentHorizontal align_h, TextAlignmentVertical align_v) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&CIwSVec2::g_Zero);

	DrawText(text.c_str(), rect, fonttype, col, align_h, align_v);
}

void Renderer::DrawText(std::string text, const CIwSVec2& pos, const CIwSVec2& size, FontType fonttype, uint32 col, TextAlignmentHorizontal align_h, TextAlignmentVertical align_v) {
	IW_CALLSTACK_SELF;

	DrawText(text, CIwRect(pos.x, pos.y, size.x, size.y), fonttype, col, align_h, align_v);
}

void Renderer::DrawText(std::string text, const CIwSVec2& pos, FontType fonttype, uint32 col, TextAlignmentHorizontal align_h, TextAlignmentVertical align_v) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&CIwSVec2::g_Zero);

	DrawText(text.c_str(), CIwRect(pos.x, pos.y, 400, 50), fonttype, col, align_h, align_v);
}

void Renderer::DrawText(std::string text, const CIwFVec2& pos, const CIwFVec2& size, FontType fonttype, uint32 col, TextAlignmentHorizontal align_h, TextAlignmentVertical align_v) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&m_xScreenOffset);

	CIwSVec2 screenpos = m_xViewport.WorldToScreen(pos);
	CIwSVec2 screensize = m_xViewport.WorldToScreen(size);
	DrawText(text.c_str(), CIwRect(screenpos.x, screenpos.y, screensize.x, screensize.y), fonttype, col, align_h, align_v);
}

void Renderer::DrawText(std::string text, const CIwFVec2& pos, FontType fonttype, uint32 col, TextAlignmentHorizontal align_h, TextAlignmentVertical align_v) {
	IW_CALLSTACK_SELF;

	IwGxSetScreenSpaceOrg(&m_xScreenOffset);

	CIwSVec2 screenpos = m_xViewport.WorldToScreen(pos);
	DrawText(text.c_str(), CIwRect(screenpos.x, screenpos.y, 400, 50), fonttype, col, align_h, align_v);
}

void Renderer::DrawText(const char* text, const CIwRect& rect, FontType font, uint32 col, TextAlignmentHorizontal align_h, TextAlignmentVertical align_v) {
	IW_CALLSTACK_SELF;

	IwGxFontAlignHor iw_align_h;
	switch (align_h) {
		case eTextAlignHLeft:
			iw_align_h = IW_GX_FONT_ALIGN_LEFT;
			break;
		case eTextAlignHCenter:
			iw_align_h = IW_GX_FONT_ALIGN_CENTRE;
			break;
		case eTextAlignHRight:
			iw_align_h = IW_GX_FONT_ALIGN_RIGHT;
			break;
		case eTextAlignHBlock:
			iw_align_h = IW_GX_FONT_ALIGN_PARAGRAPH;
			break;
	}
	IwGxFontSetAlignmentHor(iw_align_h);

	IwGxFontAlignVer iw_align_v;
	switch (align_v) {
		case eTextAlignVTop:
			iw_align_v = IW_GX_FONT_ALIGN_TOP;
			break;
		case eTextAlignVMiddle:
			iw_align_v = IW_GX_FONT_ALIGN_MIDDLE;
			break;
		case eTextAlignVBottom:
			iw_align_v = IW_GX_FONT_ALIGN_BOTTOM;
			break;
	}
	IwGxFontSetAlignmentVer(iw_align_v);
	
	if (m_apxFonts[font]) {
		IwAssertMsg(MYAPP, m_apxFonts[font], ("Cannot draw text because font %i is not set. Please load the fonts before drawing text.", font));
		IwGxFontSetFont(m_apxFonts[font]);
	}

	IwGxFontSetRect(rect);
	IwGxFontSetCol(col);

	int32 slot = IwGxGetScreenSpaceSlot();
	IwGxSetScreenSpaceSlot((int32)m_eCurrentRenderingLayer);
	IwGxFontDrawText(text);
	IwGxSetScreenSpaceSlot(slot);
}

void Renderer::DebugDrawTouch(const CIwSVec2& startpos, const CIwSVec2& endpos, uint32 col) {
#ifdef IW_DEBUG
	IW_CALLSTACK_SELF;
	DrawLine(startpos, endpos, col);
#endif
}

void Renderer::DebugDrawCoords(const CIwSVec2& point, uint32 colour, int16 size) {
#ifdef IW_DEBUG
	IW_CALLSTACK_SELF;

	std::ostringstream oss;
	oss << '(' << point.x << '/' << point.y << ')';
	DrawText(oss.str(), point, eFontTypeSystem);

	int16 len = size / 2;
	CIwSVec2 line[2] = { point, point };

	line[0] = point;
	line[0].x -= len;
	line[1] = point;
	line[1].x += len;
	DrawLine(line[0], line[1], colour);

	line[0] = point;
	line[0].y -= len;
	line[1] = point;
	line[1].y += len;
	DrawLine(line[0], line[1], colour);
#endif
}

void Renderer::DebugDrawCoords(const CIwFVec2& point, uint32 colour, float size) {
#ifdef IW_DEBUG
	IW_CALLSTACK_SELF;

	std::ostringstream oss;
	oss.precision(2);
	oss << '(' << point.x << '/' << point.y << ')';
	DrawText(oss.str(), point, eFontTypeSystem);

	uint32 cols[2] = { colour, colour };
	float len = size / 2.0f;
	CIwFVec2 line[2] = { point, point };

	line[0] = point;
	line[0].x -= len;
	line[1] = point;
	line[1].x += len;
	DrawLine(line, 2, cols);

	line[0] = point;
	line[0].y -= len;
	line[1] = point;
	line[1].y += len;
	DrawLine(line, 2, cols);
#endif
}

CIwFVec2* Renderer::CreatGxCacheVertexStream(CIwSVec2 vertices[], int count) {
	IwAssertMsg(MYAPP, count > 0 && vertices, ("Trying to allocate empty stream. This should probably never happen?"));

	CIwFVec2* polystream = IW_GX_ALLOC(CIwFVec2, count);
	for (int i = 0; i < count; i++) {
		polystream[i].x = vertices[i].x;
		polystream[i].y = vertices[i].y;
	}
	return polystream;
}

CIwFVec2* Renderer::CreatGxCacheVertexStream(CIwFVec2 vertices[], int count){
	IwAssertMsg(MYAPP, count > 0 && vertices, ("Trying to allocate empty stream. This should probably never happen?"));
	if (!vertices || count <= 0) return NULL;

	CIwFVec2* polystream = IW_GX_ALLOC(CIwFVec2, count);
	for (int i = 0; i < count; i++) {
		CIwSVec2 v = m_xViewport.WorldToScreen(vertices[i]);
		polystream[i].x = v.x;
		polystream[i].y = v.y;
	}
	return polystream;
}

CIwColour* Renderer::CreatGxCacheColourStream(uint32 cols[], int count) {
	IwAssertMsg(MYAPP, count > 0 && cols, ("Trying to allocate empty stream. This should probably never happen?"));
	if (!cols || count <= 0) return NULL;

	CIwColour* gxcols = IW_GX_ALLOC(CIwColour, count);
	//memcpy(gxcols, cols, count * sizeof(*cols));
	for (int i = 0; i < count; i++) {
		gxcols[i].Set(cols[i]);
		if (m_uiAmbientLight != RENDERER_WHITE_LIGHT) {
			CIwColour ambientlight;
			ambientlight.Set(m_uiAmbientLight);
			gxcols[i].r = gxcols[i].r * (ambientlight.r / (float)0xff);
			gxcols[i].g = gxcols[i].g * (ambientlight.g / (float)0xff);
			gxcols[i].b = gxcols[i].b * (ambientlight.b / (float)0xff);
			gxcols[i].a = gxcols[i].a * (ambientlight.a / (float)0xff);
		}
 	}
	return gxcols;
}

CIwFVec2* Renderer::CreatGxCacheUvStream(CIwFVec2 uvs[], int count) {
	IwAssertMsg(MYAPP, count > 0 && uvs, ("Trying to allocate empty stream. This should probably never happen?"));
	if (!uvs || count <= 0) return NULL;

	CIwFVec2* uvstream = IW_GX_ALLOC(CIwFVec2, count);
	for (int i = 0; i < count; i++) {
		uvstream[i] = uvs[i];
	}
	return uvstream;
}

CIwFVec2* Renderer::CreatGxCacheTiledUvStream(CIwFVec2 vertices[], int count, CIwTexture* image) {
	if (!vertices || count <= 0) return NULL;

	// get aabb
	CIwFVec2 topleft(0.0f, 0.0f), bottomright(0.0f, 0.0f);
	CalculateAABB(vertices, count, topleft, bottomright);

	// center of aabb
	int16 texturewidth = image->GetWidth();
	int16 textureheight = image->GetHeight();
	CIwSVec2 uvorigin(
		topleft.x + ((bottomright.x - topleft.x) / 2),
		topleft.y + ((bottomright.y - topleft.y) / 2));

	// align with tiles
	uvorigin.x -= uvorigin.x % texturewidth;
	uvorigin.y -= uvorigin.y % textureheight;

	// allocate UV cache and convert/rescale coordinates relative to aabb center and tile size
	CIwFVec2* uvstream = IW_GX_ALLOC(CIwFVec2, count);
	for (int i = 0; i < count; i++) {
		// TODO: Clean up for better efficiency; I just needed this for debugging
		CIwFVec2 v(vertices[i] - uvorigin);
		float x = (float) v.x;
		float y = (float) v.y;
		x /= texturewidth;
		y /= textureheight;
		uvstream[i].x = x;
		uvstream[i].y = y;
	}
	return uvstream;
}

void Renderer::CalculateAABB(CIwFVec2 vertices[], int count, CIwFVec2& topleft, CIwFVec2& bottomright) {
	// this function works in screenspace
	if (count < 1) {
		return;
	}

	topleft = vertices[0];
	bottomright = vertices[0];

	for (int i = 1; i < count; i++) {
		topleft.x = std::min<float>(vertices[i].x, topleft.x);
		topleft.y = std::min<float>(vertices[i].y, topleft.y);
		bottomright.x = std::max<float>(vertices[i].x, bottomright.x);
		bottomright.y = std::max<float>(vertices[i].y, bottomright.y);
	}
}
