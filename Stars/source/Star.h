#ifndef __STAR_H__
#define __STAR_H__

#include <queue>
#include "CompositeBody.h"
#include "ParticleSystem.h"
#include "PathTracker.h"

class Star : public CompositeBody {
public:
	class StateBase {
	protected:
		Star& m_rxContext;
		
	public:
		StateBase(Star& context) : m_rxContext(context) {};
		virtual ~StateBase() {};
		
		virtual void Initialize() {};
		virtual void FollowPath() {};

		virtual void Passify() {};
		virtual void Collide(Body& body) {};
		virtual void Update(uint16 timestep) {};
	};
		
protected:
	class InitialState;
	class PassiveState;
	class RetractingState;
	class FollowState;
	class FallingState;
	
private:
	StateBase* m_pxState;
	
	PathTracker m_xPath;
	float m_fPathSpeed;
	float m_fAnchorLine;
	bool m_bAutoOrient;

	uint32 m_uiShieldDuration;
	uint32 m_uiMagnetDuration;
	
	ParticleSystem* m_pxParticles;

public:
	Star(const std::string& id, const b2BodyDef& bodydef, const b2FixtureDef& fixturedef, const TextureTemplate& texturedef);
	virtual ~Star();

	virtual const char* GetTypeName();
	static const char* TypeName();

	void Initialize();
	
	virtual void OnUpdate(const FrameData& frame);
	virtual void OnColliding(Body& body);
	virtual void OnChildColliding(Body& child, Body& body);

	void SetAnchorLine(float xpos);
	void FollowPath(const std::vector<CIwFVec2>& path);
	
	void Passify();
	
	void BeginShield(uint32 duration);
	void EndShield(bool immediate = false);
	bool HasShield();
	
	void BeginMagnet(uint32 duration);
	void EndMagnet(bool immediate = false);
	bool HasMagnet();

	bool IsFollowingPath();

private:
	void AutoOrientTexture(bool allow);
		
	void EnableParticles();
	void DisableParticles();
	
	void OnBeginFollowPath();
	void OnEndFollowPath();
	
	void CollectNugget(const CIwFVec2& pos, int amount);
	void BeginMultiCollect(const CIwFVec2& pos);
	void CommitMultiCollect(const CIwFVec2& pos);
	void CancelMultiCollect(const CIwFVec2& pos);
	
	void SetState(StateBase* newstate);
	StateBase& GetState();
	void OnStateChanged(StateBase* oldstate, StateBase* newstate);
	
protected:
	virtual void OnRender(Renderer& renderer, const FrameData& frame);
	
public:
	MulticastEvent<Star, int> Killed;
	MulticastEvent<Star, std::string> Magnet;
};

#endif
