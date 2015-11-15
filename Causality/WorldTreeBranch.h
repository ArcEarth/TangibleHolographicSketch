#pragma once
#include "Common\Filter.h"
#include "Common\tree.h"
#include "BulletPhysics.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <map>
#include <array>
#include <deque>
#include <queue>
#include <stack>

namespace Causality
{
	typedef uint64_t ObjectIDType;

	struct ProblistiscAffineTransform : public DirectX::IsometricTransform
	{
		using DirectX::IsometricTransform::IsometricTransform;
		float Probability;
	};

	typedef std::map<unsigned, std::vector<ProblistiscAffineTransform>> SuperpositionMap;

	class WorldBranch;
	// One problistic frame for current state
	class WorldBranch : public stdx::foward_tree_node<WorldBranch, false>
	{
	public:

		enum CollisionGroupEnum : short
		{
			Group_Focused_Object = 0x1,
			Group_Unfocused_Object = 0x2,
			Group_Subject = 0x4,
			Mask_Focused_Object = 0x7,
			Mask_Unfocused_Object = 0x3,
			Mask_Subject = 0x1,
		};

	public:
		std::string												Name;

	protected:
		float													_Liklyhood;

		std::unique_ptr<std::thread>							pWorkerThread;
		std::condition_variable									queuePending;
		std::mutex												queueMutex;
	protected:
		// Evolution caculation object
		std::shared_ptr<btBroadphaseInterface>					pBroadphase = nullptr;
		// Set up the collision configuration and dispatcher
		std::shared_ptr<btDefaultCollisionConfiguration>		pCollisionConfiguration = nullptr;
		std::shared_ptr<btCollisionDispatcher>					pDispatcher = nullptr;
		// The actual physics solver
		std::shared_ptr<btSequentialImpulseConstraintSolver>	pSolver = nullptr;
		std::shared_ptr<btDynamicsWorld>						pDynamicsWorld = nullptr;

	public:
		// Object states evolution with time and interaction subjects
		std::map<std::string, std::shared_ptr<PhysicalRigid>>	Items;

		DirectX::IsometricTransform								SubjectTransform;


		bool													IsDirty;
		bool													IsEnabled;


	public:
		static void InitializeBranchPool(int size, bool autoExpandation = true);
		static std::unique_ptr<WorldBranch> DemandCreate(const std::string& branchName);
		static void Recycle(std::unique_ptr<WorldBranch>&&);

	private:
		static std::queue<std::unique_ptr<WorldBranch>> BranchPool;

	public:
		void Reset();

		float Liklyhood() const
		{
			return _Liklyhood;
		}

		WorldBranch();

		void Enable(const DirectX::IsometricTransform& subjectTransform)
		{
			IsEnabled = true;
			SubjectTransform = subjectTransform;
		}

		void Disable()
		{
			IsEnabled = false;
		}

		//copy / move sementic is deleted for use with pointer
		WorldBranch(const WorldBranch& other) = delete;
		WorldBranch& operator=(const WorldBranch& other) = delete;
		WorldBranch(WorldBranch&& other);
		WorldBranch& operator=(const WorldBranch&& other);

		void AddSubjectiveObject(const Leap::Hand& hand, const DirectX::Matrix4x4& leapTransform);
		void AddDynamicObject(const std::string &name, const std::shared_ptr<btCollisionShape> &pShape, float mass, const DirectX::Vector3 & Position, const DirectX::Quaternion & Orientation);
		void Evolution(float timeStep, const Leap::Frame & frame, const DirectX::Matrix4x4 & leapTransform);
		void Fork(const std::vector<PhysicalRigid*>& focusObjects);
		void Fork(const std::vector<DirectX::IsometricTransform>& subjectTransform);
		void Collapse();
		SuperpositionMap CaculateSuperposition();

	protected:
		float CaculateLiklyhood();
		void NormalizeLiklyhood(float total);
	public:
		// Internal evolution algorithm as-if this branch is a "Leaf"
		void InternalEvolution(float timeStep, const Leap::Frame & frame, const DirectX::Matrix4x4 & leapTransform);

	};
}