#ifndef __SPINEANIMATION_H__
#define __SPINEANIMATION_H__

#include <string>
#include "IwGeom.h"
#include "IwGx.h"
#include "spine/spine.h"

#define SPINEANIMATION_VERTS_PER_SLOT 4

class SpineAnimation {
private:
	spAtlas* m_pxAtlas;
	spSkeletonJson* m_pxSkeletonJson;
	spSkeletonData* m_pxSkeletonData;
	spSkeleton* m_pxSkeleton;
	spAnimation* m_pxAnimation;
	float m_fAnimationTime;
	
	CIwFVec2 m_xAABBLL;
	CIwFVec2 m_xAABBUR;
	
	CIwFVec2 m_xOffset;
	CIwFMat2D m_xScale;
	CIwFMat2D m_xRotation;
	CIwFMat2D m_xTranslation;
	CIwFMat2D m_xTransformation;
	
public:
	SpineAnimation();
	~SpineAnimation();
	
	void Load(const std::string& atlasfile, const std::string& jsonfile);
	void SetAnimation(const std::string& name);

	void ConfineShape(CIwFVec2 verts[], int vertcount);

	void SetPosition(const CIwFVec2& pos);
	void SetRotation(float angle);

	void Update(uint32 timestep);
	
	int GetVertexCount();
	CIwTexture* GetStreams(int length, CIwFVec2 xys[], CIwFVec2 uvs[], uint32 cols[]);

	void GetBoundigBox(CIwFVec2 bb[4]);

	void GetDebugAnimationOrigin(CIwFVec2 area[4]);
	void GetDebugAnimationOffset(CIwFVec2 area[4]);
	
private:
	void LoadSkeleton(const std::string& atlasfile, const std::string& jsonfile);
	void DestroySkeleton();

	CIwTexture* ExtractStreams(spSlot* slot, spRegionAttachment* att, int length, CIwFVec2 xys[], CIwFVec2 uvs[], uint32 cols[]);
	void ExtractUVs(CIwFVec2 uvs[4], spAtlasRegion* atlasreg);
	void ExtractColours(uint32 cols[4], spSlot* slot);
	
	bool ExtractLocalAABB(CIwFVec2& ll, CIwFVec2& ur);
	void GrowAABB(CIwFVec2& ll, CIwFVec2& ur, const CIwFVec2& point);
	float ContainAABB(const CIwFVec2& ll1, const CIwFVec2& ur1, const CIwFVec2& ll2, const CIwFVec2& ur2);
	CIwFVec2 OffsetAABB(const CIwFVec2& ll1, const CIwFVec2& ur1, const CIwFVec2& ll2, const CIwFVec2& ur2);
	
	void UpdateTransformationMatrix();
	void TransformToWorld(CIwFVec2 v[], int c);
};

#endif
