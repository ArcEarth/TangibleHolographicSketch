#pragma once
#include "Armature.h"
#include "Animations.h"
#include "ArmatureParts.h"
#include <Eigen/Core>

namespace Causality
{

	using std::vector;
	// Represent an semantic collection of animations
	// It's the possible pre-defined actions for an given object
	class BehavierSpace
	{
	public:
		typedef ArmatureFrameAnimation animation_type;
		typedef BoneHiracheryFrame frame_type;
		typedef BoneVelocityFrame velocity_frame_type;
		typedef vector<ArmatureFrameAnimation> container_type;

	private:
		IArmature*						m_pArmature;
		ShrinkedArmature				m_Parts;
		vector<animation_type>			m_AnimClips;

	public:
#pragma region Animation Clip Interfaces
		container_type& Clips() { return m_AnimClips; }
		const container_type& Clips() const { return m_AnimClips; }
		animation_type& operator[](const std::string& name);
		const animation_type& operator[](const std::string& name) const;
		void AddAnimationClip(animation_type&& animation);
		animation_type& AddAnimationClip(const std::string& name);
		bool Contains(const std::string& name) const;
#pragma endregion

		const ShrinkedArmature&	ArmatureParts() const { return m_Parts; }
		ShrinkedArmature&		ArmatureParts() { return m_Parts; }

		void					UpdateArmatureParts();

		const IArmature&		Armature() const;
		IArmature&				Armature();
		void					SetArmature(IArmature& armature);
		const frame_type&		RestFrame() const; // RestFrame should be the first fram in Rest Animation

		void					UniformQuaternionsBetweenClips();


												   //! Legacy thing ...
#pragma region Legacy Functions
											   // Using Local Position is bad : X rotation of Parent Joint is not considerd
		Eigen::VectorXf			FrameFeatureVectorEndPointNormalized(const frame_type& frame) const;
		Eigen::MatrixXf			AnimationMatrixEndPosition(const ArmatureFrameAnimation& animation) const;
		void					CacAnimationMatrixEndPosition(_In_ const ArmatureFrameAnimation& animation, _Out_ Eigen::MatrixXf& fmatrix) const;
		// Basiclly, the difference's magnitude is acceptable, direction is bad
		Eigen::VectorXf			FrameFeatureVectorLnQuaternion(const frame_type& frame) const;
		void					CaculateAnimationFeatureLnQuaternionInto(_In_ const ArmatureFrameAnimation& animation, _Out_ Eigen::MatrixXf& fmatrix) const;

		// Evaluating a likelihood of the given frame is inside this space
		float					PoseDistancePCAProjection(const frame_type& frame) const;
		Eigen::RowVectorXf		PoseSquareDistanceNearestNeibor(const frame_type& frame) const;
		Eigen::RowVectorXf		StaticSimiliartyEculidDistance(const frame_type& frame) const;

		Eigen::VectorXf			CaculateFrameDynamicFeatureVectorJointVelocityHistogram(const frame_type& frame, const frame_type& prev_frame) const;
		Eigen::RowVectorXf		DynamicSimiliarityJvh(const frame_type& frame, const frame_type& prev_frame) const;

		// with 1st order dynamic
		float FrameLikilihood(const frame_type& frame, const frame_type& prev_frame) const;
		// without dynamic
		float FrameLikilihood(const frame_type& frame) const;

		vector<ArmatureTransform> GenerateBindings();

		//float DynamicSimiliarityJvh(const velocity_frame_type& velocity_frame) const;
	protected:
		void CaculateXpInv();

		float			SegmaDis; // Distribution dervite of pose-distance

		std::vector<DirectX::Vector3> Sv; // Buffer for computing feature vector

		Eigen::VectorXf Wb; // The weights for different "Bones" , default to "Flat"

		Eigen::VectorXf X0; // Feature vector of Rest Frame
		Eigen::MatrixXf X; // Feature vector matrix = [ X1-X0 X2-X0 X3-X0 ... XN-X0]
		Eigen::MatrixXf XpInv; // PersudoInverse of X;
		Eigen::MatrixXf Xv; // Feature vector matrix for frame dyanmic = [ Xv1-Xv0 Xv2-X0 Xv3-Xv0 ... XvN-Xv0]
#pragma endregion
	};

}