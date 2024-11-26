#include "volpch.h"

#include "b3_Joint.h"
#include "b3_DistanceJoint.h"
#include "b3_BlockAllocator.h"
#include "b3_Body.h"

namespace Volcano {

	bool b3_Joint::IsEnabled() const
	{
		return m_bodyA->IsEnabled() && m_bodyB->IsEnabled();
	}

	b3_Joint* b3_Joint::Create(const b3_JointDef* def, b3_BlockAllocator* allocator)
	{
		b3_Joint* joint = nullptr;

		switch (def->type)
		{
		case e_distanceJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_DistanceJoint));
			joint = new (mem) b3_DistanceJoint(static_cast<const b3_DistanceJointDef*>(def));
		}
		break;
		/*
		case e_mouseJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_MouseJoint));
			joint = new (mem) b3_MouseJoint(static_cast<const b3_MouseJointDef*>(def));
		}
		break;

		case e_prismaticJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_PrismaticJoint));
			joint = new (mem) b3_PrismaticJoint(static_cast<const b3_PrismaticJointDef*>(def));
		}
		break;

		case e_revoluteJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_RevoluteJoint));
			joint = new (mem) b3_RevoluteJoint(static_cast<const b3_RevoluteJointDef*>(def));
		}
		break;

		case e_pulleyJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_PulleyJoint));
			joint = new (mem) b3_PulleyJoint(static_cast<const b3_PulleyJointDef*>(def));
		}
		break;

		case e_gearJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_GearJoint));
			joint = new (mem) b3_GearJoint(static_cast<const b3_GearJointDef*>(def));
		}
		break;

		case e_wheelJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_WheelJoint));
			joint = new (mem) b3_WheelJoint(static_cast<const b3_WheelJointDef*>(def));
		}
		break;

		case e_weldJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_WeldJoint));
			joint = new (mem) b3_WeldJoint(static_cast<const b3_WeldJointDef*>(def));
		}
		break;

		case e_frictionJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_FrictionJoint));
			joint = new (mem) b3_FrictionJoint(static_cast<const b3_FrictionJointDef*>(def));
		}
		break;

		case e_motorJoint:
		{
			void* mem = allocator->Allocate(sizeof(b3_MotorJoint));
			joint = new (mem) b3_MotorJoint(static_cast<const b3_MotorJointDef*>(def));
		}
		break;
		*/
		default:
			assert(false);
			break;
		}

		return joint;
	}

	void b3_Joint::Destroy(b3_Joint* joint, b3_BlockAllocator* allocator)
	{
		joint->~b3_Joint();
		switch (joint->m_type)
		{
		case e_distanceJoint:
			allocator->Free(joint, sizeof(b3_DistanceJoint));
			break;
			/*
		case e_mouseJoint:
			allocator->Free(joint, sizeof(b3_MouseJoint));
			break;

		case e_prismaticJoint:
			allocator->Free(joint, sizeof(b3_PrismaticJoint));
			break;

		case e_revoluteJoint:
			allocator->Free(joint, sizeof(b3_RevoluteJoint));
			break;

		case e_pulleyJoint:
			allocator->Free(joint, sizeof(b3_PulleyJoint));
			break;

		case e_gearJoint:
			allocator->Free(joint, sizeof(b3_GearJoint));
			break;

		case e_wheelJoint:
			allocator->Free(joint, sizeof(b3_WheelJoint));
			break;

		case e_weldJoint:
			allocator->Free(joint, sizeof(b3_WeldJoint));
			break;

		case e_frictionJoint:
			allocator->Free(joint, sizeof(b3_FrictionJoint));
			break;

		case e_motorJoint:
			allocator->Free(joint, sizeof(b3_MotorJoint));
			break;
			*/
		default:
			assert(false);
			break;
		}
	}

	b3_Joint::b3_Joint(const b3_JointDef* def)
	{
		assert(def->bodyA != def->bodyB);

		m_type = def->type;
		m_prev = nullptr;
		m_next = nullptr;
		m_bodyA = def->bodyA;
		m_bodyB = def->bodyB;
		m_index = 0;
		m_collideConnected = def->collideConnected;
		m_islandFlag = false;
		m_userData = def->userData;

		m_edgeA.joint = nullptr;
		m_edgeA.other = nullptr;
		m_edgeA.prev = nullptr;
		m_edgeA.next = nullptr;

		m_edgeB.joint = nullptr;
		m_edgeB.other = nullptr;
		m_edgeB.prev = nullptr;
		m_edgeB.next = nullptr;
	}

}