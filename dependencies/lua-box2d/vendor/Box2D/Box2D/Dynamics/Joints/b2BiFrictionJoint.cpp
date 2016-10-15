/*
* Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
* Copyright (c) 2012 Jesse van den Kieboom <jesse.vandenkieboom@epfl.ch>
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include <Box2D/Dynamics/Joints/b2BiFrictionJoint.h>
#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Dynamics/b2TimeStep.h>

void b2BiFrictionJointDef::Initialize(b2Body* body,
                                      const b2Mat22& muStatic,
                                      const b2Mat22& muKinetic)
{
	bodyA = body;
	bodyB = 0;

	this->muStatic = muStatic;
	this->muKinetic = muKinetic;
}


void b2BiFrictionJointDef::Initialize(b2Body* body,
                                      const b2Mat22& muStatic)
{
	Initialize(body, muStatic, muStatic);
}

void b2BiFrictionJointDef::Initialize(b2Body* body,
                                      const b2Vec2& muStatic,
                                      const b2Vec2& muKinetic)
{
	Initialize(body, b2Mat22(muStatic, muStatic), b2Mat22(muKinetic, muKinetic));
}

void b2BiFrictionJointDef::Initialize(b2Body* body,
                                      const b2Vec2& muStatic)
{
	Initialize(body, muStatic, muStatic);
}

b2BiFrictionJoint::b2BiFrictionJoint(const b2BiFrictionJointDef* def)
: b2Joint(def)
{
	m_linearImpulse.SetZero();

	m_muStatic = def->muStatic;
	m_muKinetic = def->muKinetic;
}

void b2BiFrictionJoint::InitVelocityConstraints(const b2SolverData& data)
{
	m_index = m_bodyA->m_islandIndex;
	m_mass = m_bodyA->m_mass;
	m_invMass = m_bodyA->m_invMass;
	
	// TODO: implement warm starting

	/*b2Vec2 vA = data.velocities[m_index].v;

	if (data.step.warmStarting)
	{
		// Scale impulses to support a variable time step.
		// m_linearImpulse *= data.step.dtRatio;
		vA -= m_invMass * m_linearImpulse;
	}
	else
	{
		m_linearImpulse.SetZero();	
	}
	
	data.velocities[m_index].v = vA;*/
}

void b2BiFrictionJoint::SolveVelocityConstraints(const b2SolverData& data)
{
	/* Static coulomb friction as impulses
	   h * mu * m * g <= m * v
	   
	   i.e.
	   h * mu * g <= v
	   
	   Thus we are going to cancel all velocities up to h * mu * g in the
	   local frame (x, y) of the body.
	*/
	
	// TODO: implement warm starting

	// Project velocity in local frame
	b2Vec2 localv = m_bodyA->GetLocalVector(data.velocities[m_index].v);
	
	// Static case
	b2Vec2 minv = data.step.dt * 9.81 * m_muStatic.ex;
	b2Vec2 maxv = data.step.dt * 9.81 * m_muStatic.ey;

	// Clamp the local velocity by the velocity restrictions imposed by
	// the friction. Note that this is currently a bit strange kind of
	// friction since it is still present when moving. I.e. dynamic friction
	// is equal to static friction at the moment
	b2Vec2 cancelv = b2Clamp(localv, -minv, maxv);
	
	// TODO: implement static and dynamic friction

	// Subtract the velocity cancelled by the friction from the total
	// velocity (after projecting the locally cancelled velocity in the
	// global frame)
	data.velocities[m_index].v -= m_bodyA->GetWorldVector(cancelv);
}

bool b2BiFrictionJoint::SolvePositionConstraints(const b2SolverData& data)
{
	B2_NOT_USED(data);

	return true;
}

b2Vec2 b2BiFrictionJoint::GetAnchorA() const
{
	return b2Vec2(0.0f, 0.0f);
}

b2Vec2 b2BiFrictionJoint::GetAnchorB() const
{
	return b2Vec2(0.0f, 0.0f);
}

b2Vec2 b2BiFrictionJoint::GetReactionForce(float32 inv_dt) const
{
	return inv_dt * m_linearImpulse;
}

float32 b2BiFrictionJoint::GetReactionTorque(float32 inv_dt) const
{
	return 0;
}

void b2BiFrictionJoint::SetMuStatic(const b2Mat22& muStatic)
{
	b2Assert(b2IsValid(muStatic.ex.x) && muStatic.ex.x >= 0.0f);
	b2Assert(b2IsValid(muStatic.ex.y) && muStatic.ex.y >= 0.0f);
	b2Assert(b2IsValid(muStatic.ey.x) && muStatic.ey.x >= 0.0f);
	b2Assert(b2IsValid(muStatic.ey.y) && muStatic.ey.y >= 0.0f);

	m_muStatic = muStatic;
}

void b2BiFrictionJoint::SetMuKinetic(const b2Mat22& muKinetic)
{
	b2Assert(b2IsValid(muKinetic.ex.x) && muKinetic.ex.x >= 0.0f);
	b2Assert(b2IsValid(muKinetic.ex.y) && muKinetic.ex.y >= 0.0f);
	b2Assert(b2IsValid(muKinetic.ey.x) && muKinetic.ey.x >= 0.0f);
	b2Assert(b2IsValid(muKinetic.ey.y) && muKinetic.ey.y >= 0.0f);

	m_muKinetic = muKinetic;
}

b2Mat22 b2BiFrictionJoint::GetMuStatic() const
{
	return m_muStatic;
}

b2Mat22 b2BiFrictionJoint::GetMuKinetic() const
{
	return m_muKinetic;
}

void b2BiFrictionJoint::Dump()
{
	int32 idx = m_bodyA->m_islandIndex;

	b2Log("  b2BiFrictionJointDef jd;\n");
	b2Log("  jd.body = bodies[%d];\n", idx);
	b2Log("  jd.collideConnected = bool(%d);\n", m_collideConnected);

	b2Log("  jd.muStatic = b2Mat22(%.15lef, %.151ef, %.15lef, %.15lef);\n",
	      m_muStatic.ex.x,
	      m_muStatic.ey.x,
	      m_muStatic.ex.y,
	      m_muStatic.ey.y);

	b2Log("  jd.muKinetic = b2Mat22(%.15lef, %.151ef, %.15lef, %.15lef);\n",
	      m_muKinetic.ex.x,
	      m_muKinetic.ey.x,
	      m_muKinetic.ex.y,
	      m_muKinetic.ey.y);

	b2Log("  joints[%d] = m_world->CreateJoint(&jd);\n", m_index);
}
