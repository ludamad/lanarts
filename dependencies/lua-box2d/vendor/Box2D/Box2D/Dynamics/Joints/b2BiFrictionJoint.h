/*
* Copyright (c) 2006-2007 Erin Catto http://www.box2d.org
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

#ifndef B2_BI_FRICTION_JOINT_H
#define B2_BI_FRICTION_JOINT_H

#include <Box2D/Dynamics/Joints/b2Joint.h>

/// Friction joint definition.
struct b2BiFrictionJointDef : public b2JointDef
{
	b2BiFrictionJointDef()
	{
		type = e_biFrictionJoint;

		muStatic.SetZero();
		muKinetic.SetZero();
	}
	
	void Initialize(b2Body* body, const b2Vec2& mu);
	void Initialize(b2Body* body, const b2Vec2& muStatic, const b2Vec2& muKinetic);
	void Initialize(b2Body* body, const b2Mat22& muStatic);
	void Initialize(b2Body* body, const b2Mat22& muStatic, const b2Mat22& muKinetic);

	// Coulomb friction coefficient
	b2Mat22 muStatic;
	b2Mat22 muKinetic;
};

/// Friction joint. This is used for top-down friction.
/// It provides 2D translational friction and angular friction.
class b2BiFrictionJoint : public b2Joint
{
public:
	b2Vec2 GetAnchorA() const;
	b2Vec2 GetAnchorB() const;

	b2Vec2 GetReactionForce(float32 inv_dt) const;
	float32 GetReactionTorque(float32 inv_dt) const;

	/// Set the static friction coefficient.
	void SetMuStatic(const b2Mat22& muStatic);

	/// Get the static friction coefficient
	b2Mat22 GetMuStatic() const;
	
	/// Set the static friction coefficient.
	void SetMuKinetic(const b2Mat22& muKinetic);

	/// Get the static friction coefficient
	b2Mat22 GetMuKinetic() const;

	/// Dump joint to dmLog
	void Dump();

protected:

	friend class b2Joint;

	b2BiFrictionJoint(const b2BiFrictionJointDef* def);

	void InitVelocityConstraints(const b2SolverData& data);
	void SolveVelocityConstraints(const b2SolverData& data);
	bool SolvePositionConstraints(const b2SolverData& data);

	// Solver shared
	b2Vec2 m_linearImpulse;
	b2Mat22 m_muStatic;
	b2Mat22 m_muKinetic;

	// Solver temp
	int32 m_index;
	float32 m_invMass;
	float32 m_mass;
};

#endif
