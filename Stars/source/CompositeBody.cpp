#include "CompositeBody.h"
#include "FactoryManager.h"
#include "Debug.h"

CompositeBody::CompositeBody(const std::string& id, const b2BodyDef& bodydef, const b2FixtureDef& fixturedef, const TextureTemplate& texturedef) 
	: Body(id, bodydef, fixturedef, texturedef) {

	m_pxParentBody = NULL;
}

CompositeBody::~CompositeBody() {
	DestroyJoints();
	DestroyChildren();
}

void CompositeBody::AddJoint(const std::string& jointid, const std::string& childa, const std::string& porta, const std::string& childb, const std::string& portb, BodyJoint::eJointType jointtype) {
	IW_CALLSTACK_SELF;

	if (m_xJointList.find(jointid) != m_xJointList.end()) {
		IwAssertMsg(MYAPP, false, ("Trying to add a joint with id '%s' to body '%s'. A joint with the same name already exists. No new joind will be added.", jointid.c_str(), GetId().c_str()));
		return;
	}
	
	// empty child id means this body (parent)
	// empty port id means origin of the body

	Body* ba = childa.empty() ? this : GetChild(childa);
	CIwFVec2* pa = ba ? ba->GetPort(porta) : &CIwFVec2::g_Zero;

	Body* bb = childb.empty() ? this : GetChild(childb);
	CIwFVec2* pb = bb ? bb->GetPort(portb) : &CIwFVec2::g_Zero;

	if (pa && ba && pb && bb) {
		if (ba != bb) {
			m_xJointList[jointid].Join(*ba, *pa, *bb, *pb, jointtype);
			AlignChildren();
			return;
		} else {
			IwAssertMsg(MYAPP, false, ("Joint in body '%s' could not be added. Port '%s'/'%s' or port %s/%s are located on the same body.",
				GetId().c_str(), childa.c_str(), porta.c_str(), childb.c_str(), portb.c_str()));
		}
	} else {
		IwAssertMsg(MYAPP, false, ("Joint in body '%s' could not be added. Port '%s'/'%s' or port %s/%s could not be found.",
			GetId().c_str(), childa.c_str(), porta.c_str(), childb.c_str(), portb.c_str()));
	}
}

void CompositeBody::RemoveJoint(const std::string&jointid) {
	JointList::iterator it = m_xJointList.find(jointid);
	if (it != m_xJointList.end()) {
		m_xJointList.erase(jointid);
	}
}

void CompositeBody::AddChild(const std::string& childid, const std::string& bodyid) {
	IW_CALLSTACK_SELF;

	Body* body = FactoryManager::GetBodyFactory().Create(bodyid);
	body->SetId(childid);

	// inheritable properties
	body->SetGravityScale(GetGravityScale());

	// no need to set position; 
	// it will be set when adding joints
	// body->SetPosition(GetPosition());
	
	AddChild(body);
}

void CompositeBody::AddChild(Body* body) {
	IW_CALLSTACK_SELF;
	if (body) {
		body->Colliding.AddListener(this, &CompositeBody::ChildCollisionHandler);
		m_xChildList[body->GetId()] = body;
	}
}

Body* CompositeBody::GetChild(const std::string& childid) {
	IW_CALLSTACK_SELF;

	for (ChildList::iterator it = m_xChildList.begin(); it != m_xChildList.end(); it++) {
		Body* body = it->second;
		if (body && body->GetId() == childid) {
			return body;
		}
	}
	return NULL;
}

void CompositeBody::RemoveChild(const std::string& childid) {
	ChildList::iterator it = m_xChildList.find(childid);
	if (it != m_xChildList.end()) {
		if (Body* sledge = it->second) {
			DestroyChild(sledge);
		}
		m_xChildList.erase(it);
	}
}

void CompositeBody::SetPosition(const CIwFVec2& position, float angle) {
	Body::SetPosition(position, angle);
	AlignChildren();
}

void CompositeBody::SetSpeed(const CIwFVec2& vector) {
	Body::SetSpeed(vector);
	
	// child bodies also need to get the speed set;
	// otherwise the parent body loses momentum
	for (ChildList::iterator it = m_xChildList.begin(); it != m_xChildList.end(); it++) {
		Body* body = it->second;
		body->SetSpeed(vector);
	}
}

void CompositeBody::EnableCollisions(bool enable, bool includechildren) {
	EnableCollisions(enable);
	if (includechildren) {
		for (ChildList::iterator it = m_xChildList.begin(); it != m_xChildList.end(); it++) {
			Body* body = it->second;
			body->EnableCollisions(enable);
		}
	}
}

void CompositeBody::EnableCollisions(bool enable) {
	Body::EnableCollisions(enable);
}

void CompositeBody::SetGravityScale(float scale) {
	Body::SetGravityScale(scale);
	for (ChildList::iterator it = m_xChildList.begin(); it != m_xChildList.end(); it++) {
		Body* body = it->second;
		body->SetGravityScale(scale);
	}
}

void CompositeBody::AlignChildren() {
	IW_CALLSTACK_SELF;

	// copy the joint list; we use it as processing queue
	typedef CIwList<BodyJoint*> JointQueue;
	JointQueue joints;
	for (JointList::iterator it = m_xJointList.begin(); it != m_xJointList.end(); it++) {
		joints.push_back(&it->second);
	}

	// Build alignment strategy
	BodyHierarchy bodies;
	while (joints.num_p > 0) {
		bool added = false;
		// try adding bodies that can be connected to already connected bodies
		JointQueue::iterator it = joints.begin();
		while (it != joints.end()) {
			if (TryAlignChild(**it, bodies, false)) {
				added = true;
				it = joints.erase(it);
			} else {
				it++;
			}
		}
		if (!added) {
			// None of the bodies can be conected; 
			// for simplicity we just pick the next available body, and 
			// condinue processing from there.
			it = joints.begin();
			IwAssertMsg(MYAPP, added, ("Composite body '%s' contains child bodies that are not connected.", GetId().c_str()));
			if (!TryAlignChild(**it, bodies, true)) {
				IwAssertMsg(MYAPP, added, ("Child in composite body '%s' cannot be aligned.", GetId().c_str()));
			}
			joints.erase(it);
		}
	}
	
	// aligning bodies according to startegy
	for (BodyHierarchy::iterator it = bodies.begin(); it != bodies.end(); it++) {
		IwDebugTraceLinePrintf("Aligning bodies '%s'->'%s' by %.2f/%.2f", (*it).child->GetId().c_str(), (*it).parent->GetId().c_str(), (*it).offset.x, (*it).offset.y);
		BodyNode& node = *it;
		node.child->SetPosition(node.parent->GetPosition() + node.offset);
	}
}

bool CompositeBody::TryAlignChild(BodyJoint& joint, BodyHierarchy& bodies, bool force) {

	// first degree joints:
	// doing this at this place is efficient if the majority of the children are 
	// expected to be attached to the composite body; however, it is inefficient for 
	// deep hierarchies of bodies,
	if (TryAlignChildAddInfo(this, joint, bodies)) {
		return true;
	}

	// higher degree joints (all other joints)
	for (BodyHierarchy::iterator it = bodies.begin(); it != bodies.end(); it++) {
		// try attaching to the child nodes first (more likely)
		if (TryAlignChildAddInfo((*it).child, joint, bodies)) {
			return true;
		}

		// also look at attaching to known parent nodes, 
		// can only hapen if disconnected children have been added
		// to the hierarchy (e.g. if there are root nodes other than *this)
		if (TryAlignChildAddInfo((*it).parent, joint, bodies)) {
			return true;
		}
	}

	// If alignment is enforced, it is likely because the node is disconnected. 
	// We have tried to add it using the correct orientation without success. 
	// We will now add it in arbitrary order. E.g. it does not matter anymore 
	// if body A or body B is parent. 
	if (force && TryAlignChildAddInfo(joint.GetBodyA(), joint, bodies)) {
		return true;
	}

	return false;
}

bool CompositeBody::TryAlignChildAddInfo(Body* parent, BodyJoint& joint, BodyHierarchy& bodies) {
	BodyNode node;
	if (joint.GetBodyA() == parent) {
		node.child = joint.GetBodyB();
		node.offset = joint.GetOffset();
	} else if (joint.GetBodyB() == parent) {
		node.child = joint.GetBodyA();
		node.offset = -joint.GetOffset();
	} else {
		return false;
	}
	node.parent = parent;
	bodies.push_back(node);
	
	return true;
}

float CompositeBody::GetMass() {
	IW_CALLSTACK_SELF;

	float mass = Body::GetMass();
	for (ChildList::iterator it = m_xChildList.begin(); it != m_xChildList.end(); it++) {
		if (Body* child = it->second) {
			mass += child->GetMass();
		}
	}
	return mass;
}

void CompositeBody::DestroyJoints() {
	IW_CALLSTACK_SELF;
	m_xJointList.clear();
}

void CompositeBody::DestroyChildren() {
	IW_CALLSTACK_SELF;
	for (ChildList::iterator it = m_xChildList.begin(); it != m_xChildList.end(); ++it) {
		if (it->second) {
			delete it->second;
		}
	}
	m_xChildList.clear();
}

void CompositeBody::DestroyChild(Body* body) {
	if (!body) {
		return;
	}

	// remove any joints that are connected to the body
	for (JointList::iterator it = m_xJointList.begin(); it != m_xJointList.end();) {
		if (it->second.GetBodyA() == body || it->second.GetBodyB() == body) {
			m_xJointList.erase(it++);
		} else {
			++it;
		}
	}
	
	body->Colliding.RemoveListener(this, &CompositeBody::ChildCollisionHandler);
	delete body;
}

void CompositeBody::OnUpdate(const FrameData& frame) {
	IW_CALLSTACK_SELF;
	for (ChildList::iterator it = m_xChildList.begin(); it != m_xChildList.end(); it++) {
		if (Body* child = it->second) {
			child->Update(frame);
		}
	}
	Body::OnUpdate(frame);
}

void CompositeBody::OnRender(Renderer& renderer, const FrameData& frame) {
	IW_CALLSTACK_SELF;
	for (ChildList::iterator it = m_xChildList.begin(); it != m_xChildList.end(); it++) {
		if (Body* child = it->second) {
			child->Render(renderer, frame);
		}
	}
	Body::OnRender(renderer, frame);
}

void CompositeBody::OnChildColliding(Body& child, Body& body) {
}

void CompositeBody::ChildCollisionHandler(const Body& sender, const Body& body) {
	// RRR: de-const
	Colliding.Invoke(*(Body*)&sender, body);
	OnChildColliding(*(Body*)&sender, *(Body*)&body);
}
