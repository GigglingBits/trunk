#ifndef __STAR_H__
#define __STAR_H__

#include <queue>
#include "Body.h"

class Star : public Body {
public:
	class StateBase {
	protected:
		Star& m_rxContext;

	public:
		StateBase(Star& context) : m_rxContext(context) {};
		virtual ~StateBase() {};

		virtual void Initialize() {};

		virtual void Stay() {};
		virtual void Jump(const CIwFVec2& impulse) {};
		virtual void Attack() {};
		virtual void Collide(Body& body) {};

		virtual void Update(uint16 timestep) {};
	};

protected:
	class IdleState;
	class JumpingState;
	class SmashingState;

private:
	StateBase* m_pxCurrentState;
	Texture* m_pxTouchTexture;

private:
	struct SchedulePoint {
		uint64 schedule;
		CIwFVec2 position;
	};
	std::queue<SchedulePoint> m_xPath;
	
public:
	Star(const std::string& id, const b2BodyDef& bodydef, const b2FixtureDef& fixturedef, const TextureTemplate& texturedef);
	virtual ~Star();

	virtual const char* GetTypeName();
	static const char* TypeName();

	virtual void OnUpdate(const FrameData& frame);
	virtual void OnColliding(Body& thisbody, Body& otherbody);

	void SetState(StateBase* newstate);
	StateBase& GetCurrentState();

	void SetTextureFrame(std::string id);

	void Jump(const CIwFVec2& impulse); 
	void Attack();
	void Sit();

	void SetPath(int samplecount, const CIwFVec2* samplepoints);
	
protected:
	virtual void OnRender(Renderer& renderer, const FrameData& frame);
};

#endif
