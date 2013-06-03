#include "Star.h"
#include "Target.h"
#include "Buff.h"
#include "StarState.h"
#include "FactoryManager.h"

/****
 * Star main implementation
 **/
Star::Star(const std::string& id, const b2BodyDef& bodydef, const b2FixtureDef& fixturedef, const TextureTemplate& texturedef) 
	: Body(id, bodydef, fixturedef, texturedef), m_pxCurrentState(NULL) {
	GetBody().SetBullet(true);
	SetState(new IdleState(*this));
	GetHealthManager().SetResilience(0.0f);
	m_pxTouchTexture = FactoryManager::GetTextureFactory().Create("touch");
}

Star::~Star() {
	if (m_pxTouchTexture) {
		delete m_pxTouchTexture;
		m_pxTouchTexture = NULL;
	}

	if (m_pxCurrentState) {
		delete m_pxCurrentState;
	}
}

const char* Star::GetTypeName() {
	return Star::TypeName();
}

const char* Star::TypeName() {
	static const char* type = "star";
	return type;
}

void Star::OnColliding(Body& thisbody, Body& otherbody) {
	IW_CALLSTACK_SELF;

	GetCurrentState().Collide(otherbody);

	if (otherbody.GetTypeName() == Buff::TypeName()) {
		GetHealthManager().RenewLife();
	} else if (otherbody.GetTypeName() == Target::TypeName()) {
		GetHealthManager().RenewLife();
	} else {
		GetHealthManager().Injure(5.0f);
	} 	
}

void Star::OnUpdate(const FrameData& frame) {
	Body::OnUpdate(frame);
	GetCurrentState().Update(frame.GetSimulatedDurationMs());

	// adjust drag force
	if (IsDragging()) {
		float distance = (GetDragTarget() - GetPosition()).GetLength();
		SetDragForce(
			(distance + 50.0f)		// the larger the distance, the larger the force to be applied
			* 2.0f					// just a constant to amplify the effect
			* GetMass());			// larger bodies require more force to move
	}
	
	// flip the texture according to movement
	if (IsDragging()) {
		// look in drag direction
		CIwFVec2 target = GetDragTarget();
		GetTexture().SetHorizontalFlip(GetPosition().x - 0.2f > target.x);
		
		//ShowEffect("smoke_plume");
	} else {
		// look in movement direction
		GetTexture().SetHorizontalFlip(GetBody().GetLinearVelocity().x <= 0.0f);
	}
	
	// prepare positin for next frame
	if (!m_xPath.empty()) {
		uint64 now = frame.GetTimeMs();
		if (m_xPath.front().schedule < now) {
			MoveDragging(m_xPath.front().position);
			m_xPath.pop();
		}
	}
}

void Star::Jump(const CIwFVec2& impulse) {
	GetCurrentState().Jump(impulse);
}

void Star::Attack() {
	GetCurrentState().Attack();
}

void Star::Sit() {
	GetCurrentState().Stay();
}

void Star::SetTextureFrame(std::string id) {
	GetTexture().SelectFrame(id, GetHealthManager().GetHealthValue());
}

void Star::OnRender(Renderer& renderer, const FrameData& frame) {
	if (IsDragging()) {
		CIwFVec2* center = GetPort("center");
		CIwFVec2 target = GetDragTarget();

		renderer.DrawLine(
			GetPosition() + (center ? *center : CIwFVec2::g_Zero),
			target,
			0xffaaaaaa);

		if (m_pxTouchTexture) {
			VertexStreamWorld verts;
			verts.SetRect(target.x - 0.2f, target.y - 0.2f, 0.4f, 0.4f);
			renderer.Draw(verts, *m_pxTouchTexture);
		}
	}

	Body::OnRender(renderer, frame);
}

void Star::SetPath(int samplecount, const CIwFVec2* samplepoints) {
	uint64 now = s3eTimerGetMs();
	for (int i = 0; i < samplecount; i++) {
		SchedulePoint schedule;
		schedule.schedule = now + i * 100; // for now, just use 20ms per sample
		schedule.position = samplepoints[i];
		m_xPath.push(schedule);
	}
}

/****
 * State machine 
 **/
void Star::SetState(Star::StateBase* newstate) {
	IwAssertMsg(MYAPP, newstate, ("Empty state must not be set."));
	if (m_pxCurrentState) {
		delete m_pxCurrentState;
	}
	newstate->Initialize();
	m_pxCurrentState = newstate;
}

Star::StateBase& Star::GetCurrentState() {
	IwAssertMsg(MYAPP, m_pxCurrentState, ("State is empty. This is a program error."));
	return *m_pxCurrentState;
}
