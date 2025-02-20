#include "Main.h"
#include "StarState.h"
#include "InputManager.h"
#include "SoundEngine.h"
#include "Nugget.h"
#include "Enemy.h"
#include "GameFoundation.h"
#include "Debug.h"

/////////////////////////////////////////////////////////////
// Initial
/////////////////////////////////////////////////////////////
void Star::InitialState::Initialize() {
}

void Star::InitialState::Update(uint16 timestep) {
	m_rxContext.Initialize();
	m_rxContext.SetState(new PassiveState(m_rxContext));
}

/////////////////////////////////////////////////////////////
// Passive
/////////////////////////////////////////////////////////////
void Star::PassiveState::Initialize() {
	m_rxContext.EnableCollisions(false, false);
	m_rxContext.SetTextureFrame("recover");
	SoundEngine::GetInstance().PlaySoundEffect("LetsGo");
	m_rxContext.DisableParticles();
	
	m_rxContext.GetBody().SetLinearDamping(5.0f);
	
	CIwFVec2 dragtarget = m_rxContext.GetDragTarget();
	dragtarget.x = m_rxContext.m_fAnchorLine;
	m_rxContext.MoveDragging(dragtarget);
	
	m_rxContext.m_bAutoOrient = true;
}

void Star::PassiveState::FollowPath() {
	m_rxContext.SetState(new FollowState(m_rxContext));
}

void Star::PassiveState::Update(uint16 timestep) {
	// balance the drag force
	if (m_rxContext.IsDragging()) {
		float distance = (m_rxContext.GetDragTarget() - m_rxContext.GetPosition()).GetLength();
		m_rxContext.SetDragForce(distance * 10.0f * m_rxContext.GetMass());
	}
}

/////////////////////////////////////////////////////////////
// Retracting
/////////////////////////////////////////////////////////////
void Star::RetractingState::Initialize() {
	m_rxContext.EnableCollisions(true, false);
	m_rxContext.SetTextureFrame("idle");
	m_rxContext.DisableParticles();
	
	m_rxContext.GetBody().SetLinearDamping(5.0f);
	
	CIwFVec2 dragtarget = m_rxContext.GetDragTarget();
	dragtarget.x = m_rxContext.m_fAnchorLine;
	m_rxContext.MoveDragging(dragtarget);
	
	m_rxContext.m_bAutoOrient = false;
}

void Star::RetractingState::Passify() {
	m_rxContext.SetState(new PassiveState(m_rxContext));
}

void Star::RetractingState::FollowPath() {
	m_rxContext.SetState(new FollowState(m_rxContext));
}

void Star::RetractingState::Collide(Body& body) {
	IW_CALLSTACK_SELF;
	if (Nugget* nugget = dynamic_cast<Nugget*>(&body)) {
		SoundEngine::GetInstance().PlaySoundEffect("EatNugget");		
		m_rxContext.CollectNugget(nugget->GetPosition(), nugget->GetDustAmount());
	} else if (!m_rxContext.HasShield()) {
		if (dynamic_cast<Enemy*>(&body) || !body.GetId().compare("screech") || !body.GetId().compare("claws")) {
			SoundEngine::GetInstance().PlaySoundEffect("Ouch");
			m_rxContext.CancelMultiCollect(body.GetPosition());
			m_rxContext.m_xPath.ClearPath();
			m_rxContext.SetState(new FallingState(m_rxContext));
		}
	} else {
		; // unhandled collision....
	}
}

void Star::RetractingState::Update(uint16 timestep) {
	// balance the drag force
	if (m_rxContext.IsDragging()) {
		float distance = (m_rxContext.GetDragTarget() - m_rxContext.GetPosition()).GetLength();
		float force = (distance > 1.0f ? 1.0f + (distance / 10.0f) : distance) * 5.0f * m_rxContext.GetMass();
		m_rxContext.SetDragForce(force);
	}
}

/////////////////////////////////////////////////////////////
// Following path
/////////////////////////////////////////////////////////////
void Star::FollowState::Initialize() {
	m_rxContext.EnableCollisions(true, false);
	m_rxContext.SetTextureFrame("followpath");
	m_rxContext.EnableParticles();
	m_rxContext.GetBody().SetLinearDamping(0.1f);
	m_rxContext.m_bAutoOrient = true;
}

void Star::FollowState::Passify() {
	m_rxContext.SetState(new PassiveState(m_rxContext));
}

void Star::FollowState::FollowPath() {
	m_rxContext.SetState(new FollowState(m_rxContext));
}

void Star::FollowState::Collide(Body& body) {
	IW_CALLSTACK_SELF;
	
	if (Nugget* nugget = dynamic_cast<Nugget*>(&body)) {
		SoundEngine::GetInstance().PlaySoundEffect("EatNugget");
		m_rxContext.CollectNugget(nugget->GetPosition(), nugget->GetDustAmount());
	} else if (!m_rxContext.HasShield()) {
		if (dynamic_cast<Enemy*>(&body) || !body.GetId().compare("screech") || !body.GetId().compare("claws")) {
			m_rxContext.SetTextureFrame("hurt");
			SoundEngine::GetInstance().PlaySoundEffect("Ouch");
			m_rxContext.CancelMultiCollect(body.GetPosition());
			m_rxContext.m_xPath.ClearPath();
			m_rxContext.SetState(new FallingState(m_rxContext));
		}
	} else {
		; // unhandled collision....
	}
}

void Star::FollowState::Update(uint16 timestep) {
	// end condition
	PathTracker& path = m_rxContext.m_xPath;
	if (!path.IsWalking()) {
		m_rxContext.SetState(new RetractingState(m_rxContext));
		return;
	}
	
	// calculate distance that can be travelled during this frame
	const float velocity = m_rxContext.m_fPathSpeed; // m/s
	float framedistance = velocity * ((float)timestep / 1000.0f);
	if (path.Walk(framedistance)) {
		m_rxContext.MoveDragging(path.GetWalkingPosition());
	}
		
	// balance the drag force
	if (m_rxContext.IsDragging()) {
		float distance = (m_rxContext.GetDragTarget() - m_rxContext.GetPosition()).GetLength();
		m_rxContext.SetDragForce(distance * 100.0f * m_rxContext.GetMass());
	}
	
	// move particles to new place
	if (m_rxContext.m_pxParticles) {
		m_rxContext.m_pxParticles->SetPosition(m_rxContext.GetPosition());
	}
}

/////////////////////////////////////////////////////////////
// Falling
/////////////////////////////////////////////////////////////
void Star::FallingState::Initialize() {
	m_rxContext.EnableCollisions(false, false);
	m_rxContext.SetTextureFrame("hurt");
	SoundEngine::GetInstance().PlaySoundEffect("StarFall");
	m_rxContext.DisableParticles();

	m_rxContext.GetBody().SetLinearDamping(1.0f);
	m_rxContext.SetDragForce(0.0f);

	m_rxContext.EnableRotation(true);
	
	m_rxContext.SetGravityScale(1.0f);
	m_rxContext.GetBody().SetLinearDamping(0.0f);

	m_rxContext.m_bAutoOrient = true;

	m_rxContext.SetImpulse(CIwFVec2(0.0f, 5.0f * m_rxContext.GetMass()), 0.5f);
	
	m_rxContext.Killed.Invoke(m_rxContext, 0);
}
