#include "pch_bcl.h"
#include "ArmatureTransforms.h"
#include <iostream>
#include "Settings.h"
#include <PrimitiveVisualizer.h>
#include "CharacterController.h"
#include "EigenExtension.h"
#include "CharacterObject.h"

#include <set>
#include <random>


using namespace std;
using namespace Causality;
using namespace ArmaturePartFeatures;
using namespace Eigen;


bool g_GestureMappingOnly = true;

extern random_device g_rand;
extern mt19937 g_rand_mt;
static std::normal_distribution<IGestureTracker::ScalarType> g_normal_dist(0,1);

extern RowVector3d	g_NoiseInterpolation;
namespace Causality
{
	extern float g_DebugArmatureThinkness;
}

typedef
Localize<EndEffector<InputFeature>>
InputExtractorType;

typedef
Pcad <
	Weighted <
	RelativeDeformation <
	AllJoints < CharacterFeature > > > >
	OutputExtractorType;


#define BEGIN_TO_END(range) range.begin(), range.end()

#ifdef _DEBUG
#define DEBUGOUT(x) std::cout << #x << " = " << x << std::endl
#else
#define DEBUGOUT(x)
#endif

template <class Derived>
void ExpandQuadraticTerm(_Inout_ DenseBase<Derived> &v, _In_ DenseIndex dim)
{
	assert(v.cols() == dim + (dim * (dim + 1) / 2));

	int k = dim;
	for (int i = 0; i < dim; i++)
	{
		for (int j = i; j < dim; j++)
		{
			v.col(k) = v.col(i).array() * v.col(j).array();
			++k;
		}
	}
}

template <typename T>
T sqr(T x)
{
	return x * x;
}


EndEffectorGblPosQuadratized::EndEffectorGblPosQuadratized()
{
}

int EndEffectorGblPosQuadratized::GetDimension(const ArmaturePart& block) const
{
	return 9;
}

RowVectorXf EndEffectorGblPosQuadratized::Get(const ArmaturePart & block, const BoneHiracheryFrame & frame)
{
	using namespace Eigen;
	RowVectorXf Y(BoneFeatureType::Dimension);

	BoneFeatures::GblPosFeature::Get(Y.segment<3>(0), frame[block.Joints.back()->ID]);

	if (block.parent() != nullptr)
	{
		RowVectorXf reference(BoneFeatures::GblPosFeature::Dimension);
		BoneFeatures::GblPosFeature::Get(reference, frame[block.parent()->Joints.back()->ID]);
		Y.segment<3>(0) -= reference;
	}

	ExpandQuadraticTerm(Y, 3);

	//Y[3] = Y[0] * Y[0];
	//Y[4] = Y[1] * Y[1];
	//Y[5] = Y[2] * Y[2];
	//Y[6] = Y[0] * Y[1];
	//Y[7] = Y[1] * Y[2];
	//Y[8] = Y[2] * Y[0];
	return Y;
}

// Inherited via IArmaturePartFeature

void EndEffectorGblPosQuadratized::Set(const ArmaturePart & block, BoneHiracheryFrame & frame, const RowVectorXf & feature)
{
	assert(false);
}

BlockizedArmatureTransform::BlockizedArmatureTransform() :pSblocks(nullptr), pTblocks(nullptr)
{
	pSource = nullptr;
	pTarget = nullptr;
}

BlockizedArmatureTransform::BlockizedArmatureTransform(const ShrinkedArmature * pSourceBlock, const ShrinkedArmature * pTargetBlock)
{
	SetFrom(pSourceBlock, pTargetBlock);
}

void BlockizedArmatureTransform::SetFrom(const ShrinkedArmature * pSourceBlock, const ShrinkedArmature * pTargetBlock)
{
	pSblocks = pSourceBlock; pTblocks = pTargetBlock;
	pSource = &pSourceBlock->Armature();
	pTarget = &pTargetBlock->Armature();
}

void BlockizedCcaArmatureTransform::Transform(frame_type & target_frame, const frame_type & source_frame) const
{
	using namespace Eigen;
	using namespace std;
	const auto& sblocks = *pSblocks;
	const auto& tblocks = *pTblocks;
	RowVectorXf X, Y;
	for (const auto& map : Maps)
	{
		if (map.Jx == -1 || map.Jy == -1) continue;

		auto pTb = tblocks[map.Jy];
		Y = pOutputExtractor->Get(*pTb, target_frame);

		if (map.Jx == -2 && map.Jy >= 0)
		{
			vector<RowVectorXf> Xs;
			int Xsize = 0;
			for (auto& pBlock : tblocks)
			{
				if (pBlock->ActiveActions.size() > 0)
				{
					Xs.emplace_back(pInputExtractor->Get(*pBlock, source_frame));
					Xsize += Xs.back().size();
				}
			}

			X.resize(Xsize);
			Xsize = 0;
			for (auto& xr : Xs)
			{
				X.segment(Xsize, xr.size()) = xr;
				Xsize += xr.size();
			}
		}
		else
		{
			auto pSb = sblocks[map.Jx];
			X = pInputExtractor->Get(*pSb, source_frame);
		}

		map.Apply(X, Y);

		//cout << " X = " << X << endl;
		//cout << " Yr = " << Y << endl;
		pOutputExtractor->Set(*pTb, target_frame, Y);

	}

	//target_frame[0].LclTranslation = source_frame[0].GblTranslation;
	//target_frame[0].LclRotation = source_frame[0].LclRotation;
	target_frame.RebuildGlobal(*pTarget);
}

void BlockizedCcaArmatureTransform::Transform(frame_type & target_frame, const frame_type & source_frame, const BoneHiracheryFrame & last_frame, float frame_time) const
{
	// redirect to pose transform
	Transform(target_frame, source_frame);
	return;

	const auto& sblocks = *pSblocks;
	const auto& tblocks = *pTblocks;
	for (const auto& map : Maps)
	{
		if (map.Jx == -1 || map.Jy == -1) continue;

		auto pSb = sblocks[map.Jx];
		auto pTb = tblocks[map.Jy];
		auto X = pInputExtractor->Get(*pSb, source_frame, last_frame, frame_time);
		auto Y = pOutputExtractor->Get(*pTb, target_frame);

		map.Apply(X, Y);

		//cout << " X = " << X << endl;
		//cout << " Yr = " << Y << endl;
		pOutputExtractor->Set(*pTb, target_frame, Y);
	}

	//target_frame[0].LclTranslation = source_frame[0].GblTranslation;
	//target_frame[0].LclRotation = source_frame[0].LclRotation;
	target_frame.RebuildGlobal(*pTarget);
}

ArmaturePartFeatures::PerceptiveVector::PerceptiveVector(CharacterController & controller)
	: m_pController(&controller)
{

}

inline int ArmaturePartFeatures::PerceptiveVector::GetDimension() const
{
	return Quadratic ? 9 : 3;
}

inline int ArmaturePartFeatures::PerceptiveVector::GetDimension(const ArmaturePart & block) const
{
	return GetDimension();
}

RowVectorXf PerceptiveVector::Get(const ArmaturePart & block, const BoneHiracheryFrame & frame)
{
	using namespace Eigen;

	DenseIndex dim = InputFeatureType::Dimension;
	if (Quadratic)
		dim += dim * (dim + 1) / 2;

	RowVectorXf Y(dim);

	auto& aJoint = *block.Joints.back();

	BoneFeatures::GblPosFeature::Get(Y.segment<3>(0), frame[aJoint.ID]);

	if (block.parent() != nullptr)
	{
		RowVectorXf reference(BoneFeatures::GblPosFeature::Dimension);
		BoneFeatures::GblPosFeature::Get(reference, frame[block.parent()->Joints.back()->ID]);
		Y.segment<3>(0) -= reference;
	}

	if (Quadratic)
	{
		ExpandQuadraticTerm(Y, InputFeatureType::Dimension);
	}

	return Y;
}

// Inherited via IArmaturePartFeature


template <class CharacterFeatureType>
Vector3f GetChainEndPositionFromY(const BoneHiracheryFrame & dFrame, const std::vector<Joint*> & joints, const RowVectorXf & Y)
{
	assert(joints.size() * CharacterFeatureType::Dimension == Y.size());

	// a double buffer
	Bone bones[2];
	int sw = 0;

	for (int i = 0; i < joints.size(); i++)
	{
		auto jid = joints[i]->ID;

		// set scale and translation
		bones[sw].LocalTransform() = dFrame[jid].LocalTransform();

		auto Yj = Y.segment<CharacterFeatureType::Dimension>(i * CharacterFeatureType::Dimension);
		// set local rotation
		CharacterFeatureType::Set(bones[sw], Yj);

		// update global data
		bones[sw].UpdateGlobalData(bones[1 - sw]);
	}

	Vector3f pos = Vector3f::MapAligned(&bones[sw].GblTranslation.x);
	return pos;
}


void PerceptiveVector::Set(const ArmaturePart & block, BoneHiracheryFrame & frame, const RowVectorXf& feature)
{
	using namespace DirectX;
	using namespace Eigen;
	if (block.ActiveActions.size() > 0)
	{
		RowVectorXf Y(CharacterFeatureType::Dimension * block.Joints.size());

		if (block.ActiveActions.size() + block.SubActiveActions.size() == 0) return;

		if (!g_UseStylizedIK)
		{
			//cpart.PdCca.Apply(feature, Y);
		}
		else
		{
			RowVectorXd dY;//(CharacterFeatureType::Dimension * cpart.Joints.size());
						   // since the feature is expanded to quadritic form, we remove the extra term
			double varZ = 10;

			//MatrixXd covObsr(g_PvDimension, g_PvDimension);
			//covObsr = g_NoiseInterpolation.replicate(1, 2).transpose().asDiagonal() * varZ;

			if (g_PvDimension == 6 && g_UseVelocity)
			{
				auto dX = feature.cast<double>().eval();
				auto& sik = m_pController->GetStylizedIK(block.Index);
				auto& gpr = sik.Gplvm();
				sik.SetBaseRotation(frame[block.parent()->Joints.back()->ID].GblRotation);
				dY = sik.Apply(dX.segment<3>(0).transpose().eval(), dX.segment<3>(3).transpose().eval());
			}
			double likilyhood = 1.0;

			//auto likilyhood = cpart.PdGpr.get_expectation_from_observation(feature.cast<double>(), covObsr, &dY);
			//auto likilyhood = cpart.PdGpr.get_expectation_and_likelihood(feature.cast<double>(), &dY);
			Y = dY.cast<float>();
			//Y *= cpart.Wx.cwiseInverse().asDiagonal(); // Inverse scale feature

			if (g_StepFrame)
				std::cout << block.Joints[0]->Name << " : " << likilyhood << std::endl;
		}

		for (size_t j = 0; j < block.Joints.size(); j++)
		{
			auto jid = block.Joints[j]->ID;
			auto Xj = Y.segment<CharacterFeatureType::Dimension>(j * CharacterFeatureType::Dimension);
			CharacterFeatureType::Set(frame[jid], Xj);
		}
	}
}

RBFInterpolationTransform::RBFInterpolationTransform(gsl::array_view<CharacterClipinfo> clips)
{
	Clipinfos = clips;
	auto pIF = new PerceptiveVector(*m_pController);
	pIF->Quadratic = true;
	pInputExtractor.reset(pIF);

	auto pOF = new PerceptiveVector(*m_pController);
	pOF->Segma = 30;
	pOutputExtractor.reset(pOF);

	pDependentBlockFeature.reset(new AllJoints<CharacterFeature>());
}

RBFInterpolationTransform::RBFInterpolationTransform(gsl::array_view<CharacterClipinfo> clips, const ShrinkedArmature * pSourceBlock, const ShrinkedArmature * pTargetBlock)
	: RBFInterpolationTransform(clips)
{
	SetFrom(pSourceBlock, pTargetBlock);
	pvs.resize(pTargetBlock->size());
}

void RBFInterpolationTransform::Render(DirectX::CXMVECTOR color, DirectX::CXMMATRIX world)
{
	using namespace DirectX;
	auto& drawer = DirectX::Visualizers::g_PrimitiveDrawer;
	drawer.SetWorld(DirectX::XMMatrixIdentity());

	//drawer.Begin();
	for (const auto& map : Maps)
	{
		auto& line = pvs[map.Jy];
		XMVECTOR p0, p1;
		p0 = XMVector3Transform(line.first.Load(), world);
		p1 = XMVector3Transform(line.second.Load(), world);
		//drawer.DrawLine(line.first, line.second, color);

		if (map.Jx >= 0)
		{

			drawer.DrawCylinder(p0, p1, g_DebugArmatureThinkness, color);
			drawer.DrawSphere(p1, g_DebugArmatureThinkness * 1.5f, color);
		}
		else
		{
			drawer.DrawSphere(p0, g_DebugArmatureThinkness * 2.0f, color);
		}
	}
	//drawer.End();
}

void RBFInterpolationTransform::Transform(frame_type & target_frame, const frame_type & source_frame) const
{
	const auto& sblocks = *pSblocks;
	const auto& tblocks = *pTblocks;

	using namespace Eigen;

	int abcount = 0;
	for (const auto& map : Maps)
	{
		//Y = X;
		if (map.Jx < 0 || map.Jy < 0) continue;

		auto pSb = sblocks[map.Jx];
		auto pTb = tblocks[map.Jy];
		auto X = pInputExtractor->Get(*pSb, source_frame);
		auto Y = pOutputExtractor->Get(*pTb, target_frame);


		map.Apply(X.leftCols<3>(), Y);

		Y.conservativeResize(9);
		ExpandQuadraticTerm(Y, 3);

		//cout << " X = " << X << endl;
		//cout << " Yr = " << Y << endl;
		pOutputExtractor->Set(*pTb, target_frame, Y);

		abcount++;
		pvs[map.Jy].second = DirectX::Vector3(Y.segment<3>(0).data());
	}

	target_frame.RebuildGlobal(*pTarget);

	// Fill Xabpv
	RowVectorXf Xabpv;
	Xabpv.resize(abcount * g_PvDimension);
	abcount = 0;
	for (const auto& map : Maps)
	{
		if (map.Jx < 0 || map.Jy < 0) continue;

		auto Yi = Xabpv.segment(abcount * g_PvDimension, g_PvDimension);

		Yi.segment<3>(0) = Vector3f::Map(&pvs[map.Jy].second.x);
		if (g_PvDimension == 9)
		{
			ExpandQuadraticTerm(Yi, 3);
		}

		abcount++;
	}

	// Dependent cparts
	for (const auto& map : Maps)
	{
		if (map.Jx == -2 && map.Jy >= 0)
		{
			if (map.uXpca.size() == Xabpv.size())
			{
				auto pTb = tblocks[map.Jy];
				auto Y = pOutputExtractor->Get(*pTb, target_frame);
				map.Apply(Xabpv, Y);
				pDependentBlockFeature->Set(*pTb, target_frame, Y);
			}
			else
			{

			}
		}
	}


	for (const auto& map : Maps)
	{
		auto pTb = tblocks[map.Jy];

		if (pTb->parent())
			pvs[map.Jy].first = target_frame[pTb->parent()->Joints.back()->ID].GblTranslation;
		pvs[map.Jy].second += pvs[map.Jy].first;
	}

}

void RBFInterpolationTransform::Transform(frame_type & target_frame, const frame_type & source_frame, const BoneHiracheryFrame & last_frame, float frame_time) const
{
	using namespace Eigen;
	using namespace DirectX;
	const auto& sblocks = *pSblocks;
	const auto& tblocks = *pTblocks;

	int abcount = 0;
	for (const auto& map : Maps)
	{
		//Y = X;
		if (map.Jx < 0 || map.Jy < 0) continue;

		auto pSb = sblocks[map.Jx];
		auto pTb = tblocks[map.Jy];
		auto X = pInputExtractor->Get(*pSb, source_frame);
		auto Xl = pInputExtractor->Get(*pSb, last_frame);
		auto Y = pOutputExtractor->Get(*pTb, target_frame);
		auto Yl = Y;

		map.Apply(X.leftCols<3>(), Y);
		map.Apply(Xl.leftCols<3>(), Yl);

		if (g_PvDimension == 9)
		{
			Y.conservativeResize(9);
			ExpandQuadraticTerm(Y, 3);
		}
		if (g_UseVelocity)
		{
			assert(Y.size() == Yl.size() && Y.size() == 3);
			Y.conservativeResize(6);
			Y.segment<3>(3) = (Y.segment<3>(0) - Yl) / (frame_time * g_FrameTimeScaleFactor);
		}

		//cout << " X = " << X << endl;
		//cout << " Yr = " << Y << endl;
		pOutputExtractor->Set(*pTb, target_frame, Y);

		abcount++;
		pvs[map.Jy].first = Vector3(Yl.data());
		pvs[map.Jy].second = Vector3(Y.segment<3>(0).data());
	}

	//target_frame.RebuildGlobal(*pTarget);

	// Fill Xabpv
	RowVectorXf Xabpv;
	Xabpv.resize(abcount * g_PvDimension);
	abcount = 0;
	for (const auto& map : Maps)
	{
		if (map.Jx < 0 || map.Jy < 0) continue;

		auto Yi = Xabpv.segment(abcount * g_PvDimension, g_PvDimension);

		Yi.segment<3>(0) = Vector3f::Map(&pvs[map.Jy].second.x);
		if (g_PvDimension == 9)
		{
			ExpandQuadraticTerm(Yi, 3);
		}
		else if (g_UseVelocity)
		{
			Yi.segment<3>(3) = Vector3f::Map(&pvs[map.Jy].first.x);
		}

		abcount++;
	}

	// Dependent cparts
	for (const auto& map : Maps)
	{
		if (map.Jx == -2 && map.Jy >= 0)
		{
			auto pTb = tblocks[map.Jy];
			RowVectorXf Y = pDependentBlockFeature->Get(*pTb, target_frame);

			if (!g_UseStylizedIK)
			{
				if (map.uXpca.size() == Xabpv.size())
				{
					map.Apply(Xabpv, Y);
					pDependentBlockFeature->Set(*pTb, target_frame, Y);
				}
			}
			else
			{
				if (pTb->SubActiveActions.size() > 0 && map.uXpca.size() == Xabpv.size())
				{
					auto _x = (Xabpv - map.uXpca) * map.pcX;

					RowVectorXd Yd;
					auto& sik = m_pController->GetStylizedIK(pTb->Index);
					auto& gpr = sik.Gplvm();
					auto lk = gpr.get_expectation_and_likelihood(_x.cast<double>(), &Yd);

					Yd.array() /= pTb->Wx.array().cast<double>();
					Y = Yd.cast<float>();// *pTb->Wx.cwiseInverse().asDiagonal();
					//Y.setOnes();

					pDependentBlockFeature->Set(*pTb, target_frame, Y);
				}
			}
		}
	}

	target_frame.RebuildGlobal(*pTarget);

	for (const auto& map : Maps)
	{
		auto pTb = tblocks[map.Jy];

		if (pTb->parent())
			pvs[map.Jy].first = target_frame[pTb->parent()->Joints.back()->ID].GblTranslation;
		pvs[map.Jy].second += pvs[map.Jy].first;
	}

}

PartilizedTransformer::PartilizedTransformer(const ShrinkedArmature& sParts, CharacterController & controller)
	: m_pHandles(nullptr), m_pController(nullptr)
{
	m_pController = &controller;
	m_pHandles = &controller.PvHandles();
	auto& parts = controller.ArmatureParts();
	auto& armature = controller.Armature();
	pSource = &sParts.Armature();
	pTarget = &armature;
	pTblocks = &parts;
	pSblocks = &sParts;

	auto pIF = std::make_shared<InputExtractorType>();
	auto pOF = std::make_shared<OutputExtractorType>();

	auto& ucinfo = controller.GetUnitedClipinfo();

	pOF->InitPcas(parts.size());
	pOF->SetDefaultFrame(armature.default_frame());
	pOF->InitializeWeights(parts);

	auto& facade = ucinfo.RcFacade;
	auto cutoff = facade.PcaCutoff();
	for (auto part : parts)
	{
		int pid = part->Index;
		if (part->ActiveActions.size() > 0 || part->SubActiveActions.size() > 0)
		{
			auto &pca = facade.GetPartPca(pid);
			auto d = facade.GetPartPcaDim(pid);
			pOF->SetPca(pid, pca.components(d), pca.mean());
		}
		else
		{
			int odim = facade.GetPartDimension(pid);
			pOF->SetPca(pid, MatrixXf::Identity(odim, odim), facade.GetPartMean(pid));
		}
	}

	m_pActiveF = pOF;
	m_pInputF = pIF;
}

void PartilizedTransformer::InitTrackers()
{
	auto& clips = m_pController->Character().Behavier().Clips();
	m_Trackers.reserve(clips.size());
	for (auto& anim : clips)
	{
		m_Trackers.emplace_back(anim, *this);
		auto& tracker = m_Trackers.back();

		//! HACK!!!
		int dim = ActiveParts.size() * m_pInputF->GetDimension() * 2;
		Eigen::RowVectorXd var(dim);
		var.setConstant(g_DefaultTrackerCovierence);
		var.segment(dim/2,dim/2) *=	g_FrameTimeScaleFactor * g_FrameTimeScaleFactor;

		tracker.SetLikihoodVarience(var.cast<IGestureTracker::ScalarType>());
		// dt = 1/30s, ds = 0.01, s = 0.3? 
		tracker.SetTrackingParameters(0.01, sqr(1.0), 0.01, sqr(0.3));
		tracker.Reset();
	}
}

void PartilizedTransformer::Transform(frame_type & target_frame, const frame_type & source_frame) const
{
	Transform(target_frame, source_frame, source_frame, .0f);
}

void PartilizedTransformer::Transform(frame_type & target_frame, const frame_type & source_frame, const BoneHiracheryFrame & last_frame, float frame_time) const
{
	bool computeVelocity = frame_time != .0f && g_UseVelocity;

	const auto& cparts = *pTblocks;
	const auto& sparts = *pSblocks;

	auto pvDim = m_pInputF->GetDimension(*sparts[0]);

	for (auto& ctrl : this->ActiveParts)
	{
		assert(ctrl.DstIdx >= 0 && ctrl.SrcIdx >= 0);
		auto& cpart = const_cast<ArmaturePart&>(*cparts[ctrl.DstIdx]);
		//auto& spart = *sparts[ctrl.SrcIdx];

		RowVectorXf xf = GetInputVector(ctrl, source_frame, last_frame, frame_time * (int)computeVelocity);
		SetHandleVisualization(cpart, xf);

		if (!m_useTracker)
			DriveActivePartSIK(cpart, target_frame, xf, computeVelocity);
	}

	if (g_EnableDependentControl && m_useTracker && DrivenParts.size() > 0)
	{
		auto& ctrl = DrivenParts[0];
		computeVelocity = true; //! HACK to force use velocity
		auto _x = GetInputVector(ctrl, source_frame, last_frame, frame_time * (int)computeVelocity).cast<IGestureTracker::ScalarType>().eval();

		auto& tracker = m_Trackers[m_currentTracker];
		auto confi = tracker.Step(_x, frame_time);
		auto ts = tracker.CurrentState();
		tracker.GetScaledFrame(target_frame, ts[0], ts[1]);
		cout << setw(6) << confi << " | " << ts << endl;
	}

	if (g_EnableDependentControl && !m_useTracker && !AccesseryParts.empty())
	{
		auto& ctrl = AccesseryParts[0];
		auto _x = GetInputVector(ctrl, source_frame, last_frame, frame_time * (int)computeVelocity);
		_x = (_x - m_pController->uXabpv) * m_pController->XabpvT;
		RowVectorXd Xd = _x.cast<double>();

		for (auto ctrl : AccesseryParts)
		{
			auto& cpart = const_cast<ArmaturePart&>(*cparts[ctrl.DstIdx]);

			DriveAccesseryPart(cpart, Xd, target_frame);
		}
	}

	target_frame[0].LclTranslation = source_frame[0].LclTranslation;
	target_frame[0].GblTranslation = source_frame[0].GblTranslation;
	target_frame.RebuildGlobal(*pTarget);
}

void PartilizedTransformer::DriveAccesseryPart(ArmaturePart & cpart, Eigen::RowVectorXd &Xd, BoneHiracheryFrame & target_frame) const
{
	auto& sik = m_pController->GetStylizedIK(cpart.Index);
	auto& gpr = sik.Gplvm();

	RowVectorXd Y;
	auto lk = gpr.get_expectation_and_likelihood(Xd, &Y);

	m_pDrivenF->Set(cpart, target_frame, Y.cast<float>());
}

void PartilizedTransformer::DriveActivePartSIK(ArmaturePart & cpart, BoneHiracheryFrame & target_frame, Eigen::RowVectorXf &xf, bool computeVelocity) const
{
	RowVectorXd Xd, Y;

	auto& sik = m_pController->GetStylizedIK(cpart.Index);
	auto& gpr = sik.Gplvm();
	auto& joints = cpart.Joints;

	auto baseRot = target_frame[cpart.parent()->Joints.back()->ID].GblRotation;
	sik.SetBaseRotation(baseRot);
	sik.SetChain(cpart.Joints, target_frame);

	Xd = xf.cast<double>();
	if (!computeVelocity)
		Y = sik.Apply(Xd.transpose());
	else
	{
		assert(xf.size() % 2 == 0);
		auto pvDim = xf.size() / 2;
		Y = sik.Apply(Xd.segment(0, pvDim).transpose(), Vector3d(Xd.segment(pvDim, pvDim).transpose()));
	}

	m_pActiveF->Set(cpart, target_frame, Y.cast<float>());

	for (int i = 0; i < joints.size(); i++)
		target_frame[joints[i]->ID].UpdateGlobalData(target_frame[joints[i]->parent()->ID]);

}

void PartilizedTransformer::SetHandleVisualization(ArmaturePart & cpart, Eigen::RowVectorXf &xf) const
{
	if (m_pHandles)
	{
		m_pHandles->at(cpart.Index).first = Vector3(xf.data());
		if (g_UseVelocity && g_PvDimension == 6)
		{
			m_pHandles->at(cpart.Index).second = Vector3(xf.data() + 3);
		}
		else
		{
			m_pHandles->at(cpart.Index).second = Vector3::Zero;
		}
	}
}

void PartilizedTransformer::TransformCtrlHandel(RowVectorXf &xf, const Eigen::MatrixXf& homo)
{
	xf *= homo.topLeftCorner(homo.rows() - 1, homo.cols() - 1);
	xf += homo.block(homo.rows() - 1, 0, 1, homo.cols() - 1);
}

using namespace std;

void PartilizedTransformer::GenerateDrivenAccesseryControl()
{
	auto& allclip = m_pController->GetUnitedClipinfo();
	auto& controller = *m_pController;

	int pvDim = allclip.PvFacade.GetAllPartDimension();

	// Build aparts, dparts
	VectorXi aparts(this->ActiveParts.size());
	set<int> drivSet(BEGIN_TO_END(allclip.ActiveParts()));
	for (int i = 0; i < this->ActiveParts.size(); i++)
	{
		aparts[i] = this->ActiveParts[i].DstIdx;
		drivSet.erase(aparts[i]);
	}
	VectorXi dparts(drivSet.size());
	int i = 0;
	for (auto& pid : drivSet)
		dparts[i++] = pid;

	// Select Xapvs & dpavs
	MatrixXf Xapvs(allclip.ClipFrames(), pvDim * aparts.size());
	MatrixXi cache = aparts.replicate(1, 3).transpose();
	selectCols(allclip.PvFacade.GetAllPartsSequence(), VectorXi::Map(cache.data(), cache.size()), &Xapvs);

	//MatrixXf Xdpvs(allclip.ClipFrames(), pvDim * dparts.size());
	//cache = dparts.replicate(1, 3).transpose();
	//selectCols(allclip.PvFacade.GetAllPartsSequence(), VectorXi::Map(cache.data(), cache.size()), &Xdpvs);

	Pca<MatrixXf> pcaXapvs(Xapvs);
	int dXapv = pcaXapvs.reducedRank(g_CharacterPcaCutoff);
	QrStore<MatrixXf> qrXabpvs(pcaXapvs.coordinates(dXapv));
	Cca<float> cca;

	PcaCcaMap map;
	DrivenParts.clear();
	for (int i = 0; i < dparts.size(); i++)
	{
		auto pid = dparts[i];
		DrivenParts.emplace_back();
		auto &ctrl = DrivenParts.back();
		ctrl.DstIdx = pid;
		ctrl.SrcIdx = PvInputTypeEnum::ActiveParts;

		int d = allclip.PvFacade.GetPartPcaDim(pid);
		auto& pca = allclip.PvFacade.GetPartPca(pid);
		auto qr = allclip.PvFacade.GetPartPcadQrView(pid);

		cca.computeFromQr(qrXabpvs, qr, true, 0);
		float corr = cca.correlaltions().minCoeff();

		map.A = cca.matrixA();
		map.B = cca.matrixB();
		map.useInvB = false;
		map.svdBt = Eigen::JacobiSVD<Eigen::MatrixXf>(map.B.transpose(), Eigen::ComputeThinU | Eigen::ComputeThinV);
		map.uX = qrXabpvs.mean();
		map.uY = qr.mean();
		map.pcX = pcaXapvs.components(dXapv);
		map.uXpca = pcaXapvs.mean();
		map.pcY = pca.components(d);
		map.uYpca = pca.mean();

		ctrl.HomoMatrix = map.TransformMatrix();
	}
}

void PartilizedTransformer::EnableTracker(int whichTracker)
{
	m_currentTracker = whichTracker;
	m_useTracker = true;

	if (m_currentTracker >= m_Trackers.size() || m_currentTracker < 0)
	{
		m_currentTracker = -1;
		m_useTracker = false;
	}
}

void PartilizedTransformer::EnableTracker(const std::string & animName)
{
	auto& clips = m_pController->Character().Behavier().Clips();
	int which = std::find_if(BEGIN_TO_END(clips), [&animName](const ArmatureFrameAnimation& anim) { return anim.Name == animName;}) - clips.begin();
	EnableTracker(which);
}

void PartilizedTransformer::ResetTrackers()
{
	for (auto& tracker : m_Trackers)
		tracker.Reset();
}

RowVectorXf PartilizedTransformer::GetInputVector(const P2PTransform& Ctrl, const frame_type & source_frame, const BoneHiracheryFrame & last_frame, float frame_time) const
{
	bool computeVelocity = frame_time != .0f;
	const auto& sparts = *pSblocks;
	auto& feature = *m_pInputF;

	int pvDim = feature.GetDimension();
	RowVectorXf Xd;

	if (Ctrl.SrcIdx >= 0)
	{
		auto& spart = *sparts[Ctrl.SrcIdx];
		pvDim = feature.GetDimension(spart);

		Xd.resize(computeVelocity ? pvDim * 2 : pvDim);

		RowVectorXf xf = feature.Get(spart, source_frame);//,last_frame,frame_time

		TransformCtrlHandel(xf, Ctrl.HomoMatrix);

		Xd.segment(0, pvDim) = xf;

		if (computeVelocity)
		{
			RowVectorXf xlf = feature.Get(spart, last_frame);
			TransformCtrlHandel(xlf, Ctrl.HomoMatrix);
			auto Xv = Xd.segment(pvDim, pvDim);
			Xv = (xf - xlf) / (frame_time * g_FrameTimeScaleFactor);
		}
	}
	else if (Ctrl.SrcIdx == PvInputTypeEnum::ActiveParts)
	{
		if (pvDim > 0)
		{
			pvDim <<= (int)computeVelocity;
			Xd.resize(pvDim * ActiveParts.size());
			for (int i = 0; i < ActiveParts.size(); i++)
			{
				auto& actrl = ActiveParts[i];
				Xd.segment(i * pvDim, pvDim) = GetInputVector(actrl, source_frame, last_frame, frame_time);
			}
		}
		else
		{
			throw std::logic_error("Input feature must be constant dimensional feature");
		}
	}

	return Xd;
	// TODO: insert return statement here
}


CharacterActionTracker::CharacterActionTracker(const ArmatureFrameAnimation & animation, const PartilizedTransformer &transfomer)
	: m_Animation(animation), m_Transformer(transfomer), m_confidentThre(0.001), m_uS(1.0), m_uVt(0.0)
{}

void CharacterActionTracker::Reset(const InputVectorType & input)
{
	Reset();
	SetInputState(input, 1.0f/30.0f);
	StepParticals();
}

void CharacterActionTracker::Reset()
{
	auto& frames = m_Animation.GetFrameBuffer();

	m_dt = m_Animation.FrameInterval.count() / 2;
	auto tchuck = frames.size() / 2;
	auto dt = m_dt;

	RowVector4f v;
	const int schunck = 3;
	const int vchunck = 3;
	m_sample.resize(tchuck * schunck * vchunck, 4 + 1);
	auto stdevS = sqrt(m_varS);
	auto stdevV = sqrt(m_varVt);

	v[3] = .0f;
	for (int i = 0; i < tchuck; i++)
	{
		v[0] = i * dt;
		float s = m_uS - stdevS;
		for (int j = 0; j < schunck; j++, s += 2 * stdevS / (schunck - 1))
		{
			v[1] = s;
			float vt = m_uVt - stdevV;
			for (int k = 0; k < vchunck; k++, vt += 2 * stdevV / (vchunck - 1))
			{
				v[2] = vt;
				m_sample.block<1, 4>(i * schunck * vchunck + j * vchunck + k, 1) = v;
			}
		}
	}

	m_sample.col(0).array() = 1.0f;

	m_newSample.resizeLike(m_sample);

}

CharacterActionTracker::ScalarType CharacterActionTracker::Step(const InputVectorType & input, ScalarType dt)
{
	SetInputState(input, dt);
	auto confi = StepParticals();
	if (confi < m_confidentThre)
	{
		cout << "[Tracker] *Rest*************************" << endl;
		Reset(input);
		confi = m_sample.col(0).sum();
	}
	return confi;
}

void CharacterActionTracker::SetInputState(const InputVectorType & input, ScalarType dt)
{
	m_CurrentInput = input;
	m_dt = dt;
}

CharacterActionTracker::ScalarType CharacterActionTracker::Likilihood(const TrackingVectorBlockType & x)
{
	auto vx = GetCorrespondVector(x);

	// Distance to observation
	InputVectorType diff = (vx - m_CurrentInput).cwiseAbs2().eval();
	ScalarType likilihood = (diff.array() / m_LikCov.array()).sum();
	likilihood = exp(-likilihood);

	// Scale factor distribution
	likilihood *= exp(-sqr(x[1] - m_uS) / m_varS);
	likilihood *= exp(-sqr(x[2] - m_uVt) / m_varVt);

	//return 1.0;
	return likilihood;
}

void CharacterActionTracker::Progate(TrackingVectorBlockType & x)
{
	auto& t = x[0];
	auto& s = x[1];
	auto& vt = x[2];
	auto& ds = x[3];

	vt += g_normal_dist(g_rand_mt) * m_stdevDVt;
	auto dt = vt * m_dt;
	// ds += ;

	t += dt;
	s += g_normal_dist(g_rand_mt) * m_stdevDs;
}

CharacterActionTracker::InputVectorType CharacterActionTracker::GetCorrespondVector(const TrackingVectorBlockType & x) const
{
	InputVectorType vout;
	bool has_vel = false;
	if (x.size() == 4)
		has_vel = true;

	float t = x[0], s = x[1];

	P2PTransform ctrl;
	ctrl.SrcIdx = PvInputTypeEnum::ActiveParts;

	GetScaledFrame(m_Frame, t, s);

	if (!has_vel)
	{
		vout = m_Transformer.GetInputVector(ctrl, m_Frame, m_Frame, .0f).cast<ScalarType>();
	}
	else
	{
		float dt = x[2] * m_dt, ds = x[3];
		GetScaledFrame(m_LastFrame, t - dt, s);

		vout = m_Transformer.GetInputVector(ctrl, m_Frame, m_LastFrame, dt).cast<ScalarType>();
	}
	return vout;
}

void CharacterActionTracker::GetScaledFrame(_Out_ BoneHiracheryFrame& frame, ScalarType t, ScalarType s) const
{
	m_Animation.GetFrameAt(frame, time_seconds(t));
	ScaleFrame(frame, m_Animation.DefaultFrame(), s);
	frame.RebuildGlobal(m_Animation.Armature());
}

void CharacterActionTracker::SetLikihoodVarience(const InputVectorType & v)
{
	m_LikCov = v;
}

void CharacterActionTracker::SetTrackingParameters(ScalarType stdevDVt, ScalarType varVt, ScalarType stdevDs, ScalarType varS)
{
	m_stdevDVt = stdevDVt;
	m_varVt = varVt;
	m_stdevDs = stdevDs;
	m_varS = varS;
}
