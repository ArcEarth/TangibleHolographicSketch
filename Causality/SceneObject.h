#pragma once
#include <memory>
#include "Math3D.h"
#include <HierarchicalTransform.h>
#include <Locatable.h>
#include <chrono>
#include "String.h"
#include "Common\tree.h"

namespace Causality
{
	using time_seconds = std::chrono::duration<double>;

	using DirectX::IRigid;
	using stdx::tree_node;

	class Scene;
	class Component;

	enum SceneObjectCollisionType
	{
		Collision_Dynamic,	// Passive, Collision with Static and Kinametic Object
		Collision_Static,		// Passive, Collision with Dynamic Type, won't move
		Collision_Kinametic,	// Active, Collision with Dynamic Type, will move
		Collision_Ghost,		// Active, Don't Collide, but will move
	};

	//struct ProblistiscAffineTransform : public IsometricTransform
	//{
	//	using IsometricTransform::IsometricTransform;
	//	float Probability;
	//};

	//typedef std::vector<ProblistiscAffineTransform> SuperPosition;

#pragma region AbstractObject
	//class AbstractObject : public tree_node<SceneObject>
	//{
	//	typedef tree_node<AbstractObject> tree_base_type;

	//	template <typename T>
	//	bool Is() const
	//	{
	//		return dynamic_cast<const T*>(this) != nullptr;
	//	}

	//	template <typename T>
	//	const T* As() const
	//	{
	//		return dynamic_cast<const T*>(this);
	//	}

	//	template <typename T>
	//	T* As()
	//	{
	//		return dynamic_cast<T*>(this);
	//	}

	//	virtual ~AbstractObject();

	//	AbstractObject();

	//	virtual void AddChild(AbstractObject* child);

	//	virtual void OnParentChanged(AbstractObject* oldParent);

	//	template <typename T>
	//	T* FirstAncesterOfType()
	//	{
	//		auto node = parent();
	//		while (node && !node->Is<T>())
	//			node = node->parent();
	//		if (node)
	//			return node->As<T>();
	//		else
	//			return nullptr;
	//	}
	//	template <typename T>
	//	const T* FirstAncesterOfType() const
	//	{
	//		auto node = parent();
	//		while (node && !node->Is<T>())
	//			node = node->parent();
	//		if (node)
	//			return node->As<T>();
	//		else
	//			return nullptr;
	//	}

	//	template <typename T>
	//	T* FirstChildOfType()
	//	{
	//		auto node = first_child();
	//		while (node && !node->Is<T>())
	//			node = node->next_sibling();
	//		if (node)
	//			return node->As<T>();
	//		else
	//			return nullptr;
	//	}

	//	template <typename T>
	//	const T* FirstChildOfType() const
	//	{
	//		auto node = first_child();
	//		while (node && !node->Is<T>())
	//			node = node->next_sibling();
	//		if (node)
	//			return node->As<T>();
	//		else
	//			return nullptr;
	//	}

	//	template <typename TInterface>
	//	auto DescendantsOfType()
	//	{
	//		using namespace boost;
	//		using namespace adaptors;
	//		return
	//			descendants()
	//			| transformed([](auto pCom) {
	//			return dynamic_cast<TInterface*>(pCom); })
	//			| filtered([](auto pCom) {
	//				return pCom != nullptr;});
	//	}

	//	virtual void Update(time_seconds const& time_delta);

	//};
#pragma endregion

	// Basic class for all object, camera, entity, or light
	// It also holds a Axis-Aligned bounding box for each node, thus a AABB-tree
	XM_ALIGNATTR
	class SceneObject : public tree_node<SceneObject>, virtual public IRigid , public AlignedNew<XMVECTOR>
	{
	public:
		typedef tree_node<SceneObject> tree_base_type;
		typedef IsometricTransform TransformType;

		template <typename T>
		bool Is() const
		{
			return dynamic_cast<const T*>(this) != nullptr;
		}

		template <typename T>
		const T* As() const
		{
			return dynamic_cast<const T*>(this);
		}

		template <typename T>
		T* As()
		{
			return dynamic_cast<T*>(this);
		}

		virtual ~SceneObject();

		SceneObject();

		virtual void AddChild(SceneObject* child);

		virtual void OnParentChanged(SceneObject* oldParent);

		template <typename T>
		T* FirstAncesterOfType()
		{
			auto node = parent();
			while (node && !node->Is<T>())
				node = node->parent();
			if (node)
				return node->As<T>();
			else
				return nullptr;
		}
		template <typename T>
		const T* FirstAncesterOfType() const
		{
			auto node = parent();
			while (node && !node->Is<T>())
				node = node->parent();
			if (node)
				return node->As<T>();
			else
				return nullptr;
		}

		template <typename T>
		T* FirstChildOfType()
		{
			auto node = first_child();
			while (node && !node->Is<T>())
				node = node->next_sibling();
			if (node)
				return node->As<T>();
			else
				return nullptr;
		}

		template <typename T>
		const T* FirstChildOfType() const
		{
			auto node = first_child();
			while (node && !node->Is<T>())
				node = node->next_sibling();
			if (node)
				return node->As<T>();
			else
				return nullptr;
		}

		template <typename TInterface>
		auto DescendantsOfType() 
		{
			using namespace boost;
			using namespace adaptors;
			return 
				descendants()
				| transformed([](auto pCom) {
					return dynamic_cast<TInterface*>(pCom); }) 
				| filtered([](auto pCom){
					return pCom != nullptr;});
		}

		virtual void						Update(time_seconds const& time_delta);

		bool								IsStatic() const { return m_IsStatic; }

		bool								IsEnabled() const { return m_IsEnabled; }
		void								SetEnabled(bool enabled = true) { m_IsEnabled = enabled; }

		// Hierachical Transform Management
		XMMATRIX							GlobalTransformMatrix() const;

		void XM_CALLCONV					Move(FXMVECTOR p);
		void XM_CALLCONV					Rotate(FXMVECTOR q);

		virtual Vector3						GetPosition() const override;
		virtual Quaternion                  GetOrientation() const override;
		virtual Vector3                     GetScale() const override;

		virtual void						SetPosition(const Vector3 &p) override;
		virtual void                        SetOrientation(const Quaternion &q) override;
		virtual void                        SetScale(const Vector3 & s) override;

		void								SetLocalTransform(const TransformType& lcl);
		const TransformType&				GetGlobalTransform() const;

		virtual bool						GetBoundingBox(BoundingBox& box) const;
		virtual bool						GetBoundingGeometry(BoundingGeometry& geometry) const;

	protected:
		void								SetTransformDirty();
		void								UpdateTransformsParentWard() const;
		void								UpdateTransformsChildWard();

	public:
		string								Name;
		string								Tag;
		Scene								*Scene;

	protected:
		mutable HierachicalTransform		m_Transform;
		mutable bool						m_TransformDirty;
		bool								m_IsStatic;
		bool								m_IsEnabled;

	};
}