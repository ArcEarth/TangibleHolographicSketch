#pragma once
#include "Animations.h"
#include "CCA.h"
#include <vector>
#include <map>
#include "RegressionModel.h"
#include "PcaCcaMap.h"
#include "ArmatureParts.h"
#include "BoneFeatures.h"
#include "ArmaturePartFeatures.h"
#include "GestureTracker.h"

namespace Causality
{
	struct TransformPair
	{
		int Jx, Jy;
		std::unique_ptr<IRegression> pRegression;
	};

	class CharacterController;

	namespace ArmaturePartFeatures
	{
		class EndEffectorGblPosQuadratized : public IArmaturePartFeature
		{
		public:
			EndEffectorGblPosQuadratized();

			int GetDimension(_In_ const ArmaturePart& block) const override;

			typedef BoneFeatures::QuadraticGblPosFeature BoneFeatureType;
			virtual Eigen::RowVectorXf Get(_In_ const ArmaturePart& block, _In_ const BoneHiracheryFrame& frame) override;

			// Inherited via IArmaturePartFeature
			virtual void Set(const ArmaturePart & block, BoneHiracheryFrame & frame, const Eigen::RowVectorXf & feature) override;
		};

		class AllJointRltLclRotLnQuatPcad : public Pcad<RelativeDeformation<AllJoints<BoneFeatures::LclRotLnQuatFeature>>>
		{
		public:
			typedef Pcad<RelativeDeformation<AllJoints<BoneFeatures::LclRotLnQuatFeature>>> BaseType;


		};
	}

	class BlockizedArmatureTransform : public ArmatureTransform
	{
	protected:
		const ShrinkedArmature *pSblocks, *pTblocks;
	public:

		BlockizedArmatureTransform();

		BlockizedArmatureTransform(const ShrinkedArmature * pSourceBlock, const ShrinkedArmature * pTargetBlock);

		void SetFrom(const ShrinkedArmature * pSourceBlock, const ShrinkedArmature * pTargetBlock);
	};

	class BlockizedCcaArmatureTransform : public BlockizedArmatureTransform
	{
	public:
		std::vector<PcaCcaMap> Maps;

		std::unique_ptr<IArmaturePartFeature> pInputExtractor, pOutputExtractor;
	public:

		using BlockizedArmatureTransform::BlockizedArmatureTransform;

		virtual void Transform(_Out_ frame_type& target_frame, _In_ const frame_type& source_frame) const override;

		virtual void Transform(_Out_ frame_type& target_frame, _In_ const frame_type& source_frame, _In_ const BoneHiracheryFrame& last_frame, float frame_time) const;
	};

	namespace ArmaturePartFeatures
	{
		class PerceptiveVector : public IArmaturePartFeature
		{
		private:
			CharacterController			*m_pController;
		public:
			float						Segma;
			bool						Quadratic;
			bool						Velocity;
		public:

			typedef BoneFeatures::GblPosFeature InputFeatureType;
			typedef CharacterFeature CharacterFeatureType;

			PerceptiveVector(CharacterController& controller);

			virtual int GetDimension() const;

			virtual int GetDimension(_In_ const ArmaturePart& block) const override;

			virtual Eigen::RowVectorXf Get(_In_ const ArmaturePart& block, _In_ const BoneHiracheryFrame& frame) override;

			// Inherited via IArmaturePartFeature
			virtual void Set(_In_ const ArmaturePart& block, _Out_ BoneHiracheryFrame& frame, _In_ const Eigen::RowVectorXf& feature) override;
		};
	}

	class CharacterClipinfo;

	class RBFInterpolationTransform : public BlockizedCcaArmatureTransform
	{
	private:
		CharacterController			*m_pController;

	public:
		RBFInterpolationTransform(gsl::array_view<CharacterClipinfo> clips);

		RBFInterpolationTransform(gsl::array_view<CharacterClipinfo> clips, const ShrinkedArmature * pSourceBlock, const ShrinkedArmature * pTargetBlock);

		gsl::array_view<CharacterClipinfo>		Clipinfos;

		mutable std::vector<std::pair<DirectX::Vector3, DirectX::Vector3>> pvs;
		std::unique_ptr<IArmaturePartFeature> pDependentBlockFeature;

		void Render(DirectX::CXMVECTOR color, DirectX::CXMMATRIX world);

		virtual void Transform(_Out_ frame_type& target_frame, _In_ const frame_type& source_frame) const override;

		virtual void Transform(_Out_ frame_type& target_frame, _In_ const frame_type& source_frame, _In_ const BoneHiracheryFrame& last_frame, float frame_time) const override;
	};

	// for >=0, to sepecific part
	enum PvInputTypeEnum
	{
		ActiveAndDrivenParts = -2,
		ActiveParts = -3,
		NoInputParts = -1,
	};

	//Part to Part transform
	struct P2PTransform
	{
		int DstIdx, SrcIdx;
		Eigen::MatrixXf HomoMatrix; // homogenians transfrom matrix
	};

	class CharacterController;

	class PartilizedTransformer;

	class CharacterActionTracker : public ParticaleFilterBase
	{
	public:
		CharacterActionTracker(const ArmatureFrameAnimation& animation,const PartilizedTransformer &transfomer);
		// Inherited via ParticaleFilterBase
	public:
		virtual void	Reset(const InputVectorType & input) override;

		void			Reset();

		ScalarType		Step(const InputVectorType& input, ScalarType dt) override;

		void			GetScaledFrame(_Out_ BoneHiracheryFrame& frame, ScalarType t, ScalarType s) const;

		void			SetLikihoodVarience(const InputVectorType& v);

		void			SetTrackingParameters(ScalarType stdevDVt, ScalarType varVt, ScalarType stdevDs, ScalarType varS);
	protected:
		void			SetInputState(const InputVectorType & input, ScalarType dt) override;
		ScalarType		Likilihood(const TrackingVectorBlockType & x) override;
		void			Progate(TrackingVectorBlockType & x) override;

		InputVectorType GetCorrespondVector(const TrackingVectorBlockType & x) const;

	protected:
		const ArmatureFrameAnimation&	m_Animation;
		const PartilizedTransformer&	m_Transformer;

		mutable BoneHiracheryFrame		m_Frame;
		mutable BoneHiracheryFrame		m_LastFrame;
		InputVectorType					m_CurrentInput;
		std::shared_ptr<IArmaturePartFeature>	m_pFeature;

		// Likilihood distance cov 
		InputVectorType					m_LikCov;
		ScalarType						m_dt;
		ScalarType						m_confidentThre;

		// Progation velocity variance
		ScalarType						m_stdevDVt;
		ScalarType						m_varVt;
		ScalarType						m_stdevDs;
		ScalarType						m_varS;
		ScalarType						m_uS;
		ScalarType						m_uVt;
	};

	class PartilizedTransformer : public BlockizedArmatureTransform
	{
	public:
		std::vector<P2PTransform> ActiveParts;	// Direct controlled by input armature, with stylized IK

		PartilizedTransformer(const ShrinkedArmature& sParts, CharacterController & controller);

		void InitTrackers();

		virtual void Transform(_Out_ frame_type& target_frame, _In_ const frame_type& source_frame) const override;

		virtual void Transform(_Out_ frame_type& target_frame, _In_ const frame_type& source_frame, _In_ const BoneHiracheryFrame& last_frame, float frame_time) const override;

		void DriveAccesseryPart(Causality::ArmaturePart & cpart, Eigen::RowVectorXd &Xd, Causality::BoneHiracheryFrame & target_frame) const;

		void DriveActivePartSIK(Causality::ArmaturePart & cpart, Causality::BoneHiracheryFrame & target_frame, Eigen::RowVectorXf &xf, bool computeVelocity = false) const;

		void SetHandleVisualization(Causality::ArmaturePart & cpart, Eigen::RowVectorXf &xf) const;

		static void TransformCtrlHandel(Eigen::RowVectorXf &xf, const Eigen::MatrixXf& homoMatrix);

		void GenerateDrivenAccesseryControl();

		void EnableTracker(int whichTracker);
		void EnableTracker(const std::string& animName);

		void ResetTrackers();

	public:
		std::vector<P2PTransform> DrivenParts;	// These parts will be drive by active parts on Cca base than stylized IK
		std::vector<P2PTransform> AccesseryParts; // These parts will be animated based on active parts (and driven parts?)

		typedef Eigen::RowVectorXf InputVectorType;
		InputVectorType GetInputVector(_In_ const P2PTransform& Ctrl, _In_ const frame_type& source_frame, _In_ const BoneHiracheryFrame& last_frame, _In_ float frame_time) const;
	private:
		typedef std::pair<DirectX::Vector3, DirectX::Vector3> LineSegment;

		CharacterController			*m_pController;
		std::vector<LineSegment>	*m_pHandles;

		typedef std::shared_ptr<IArmaturePartFeature> FeaturePtr;

		bool		m_useTracker;

		mutable
		FeaturePtr	m_pInputF;
		mutable
		FeaturePtr	m_pActiveF;
		mutable
		FeaturePtr	m_pDrivenF;
		mutable
		FeaturePtr	m_pAccesseryF;

		int			m_currentTracker;
		// Tracker for different animations 
		mutable
		std::vector<CharacterActionTracker>
					m_Trackers;
	};
}