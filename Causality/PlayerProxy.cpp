#include "pch_bcl.h"
#include <PrimitiveVisualizer.h>
#include <fstream>
#include <algorithm>
#include <ppl.h>
#include <boost\filesystem.hpp>
//#include <random>
//#include <unsupported\Eigen\fft>
//#pragma warning (disable:4554)
//#include <unsupported\Eigen\CXX11\Tensor>
#include "CCA.h"
#include "EigenExtension.h"
#include "GaussianProcess.h"
//#include "QudraticAssignment.h"

#include "ArmatureParts.h"
#include "ClipMetric.h"

#include "PlayerProxy.h"
#include "Scene.h"
#include "ArmatureTransforms.h"
#include "Settings.h"
#include "CameraObject.h"
#include <Models.h>

//					When this flag set to true, a CCA will be use to find the general linear transform between Player 'Limb' and Character 'Limb'

//float				g_NoiseInterpolation = 1.0f;


using namespace Causality;
using namespace Eigen;
using namespace std;
using namespace ArmaturePartFeatures;
using boost::filesystem::path;

path g_LogRootDir = "Log";
static const char*  DefaultAnimationSet = "walk";
Eigen::RowVector3d	g_NoiseInterpolation = { 1.0,1.0,1.0 };
const static Eigen::IOFormat CSVFormat(StreamPrecision, DontAlignCols, ", ", "\n");

ShrinkedArmature		 g_PlayerParts;
std::map<string, string> g_DebugLocalMotionAction;
bool					 g_DebugLocalMotion = false;
bool					 g_ForceRemappingAlwaysOn = false;

static const DirectX::XMVECTORF32 HumanBoneColors[JointType_Count] = {
	{ 0.0f,0.0f,0.0f,0.0f }, //JointType_SpineBase = 0,
	{ 0.0f,0.0f,0.0f,0.0f },//JointType_SpineMid = 1,
	{ 0.9f,0.3f,0.9f,1.0f },//JointType_Neck = 2,
	{ 0.9f,0.3f,0.9f,1.0f },//JointType_Head = 3,
	{ 0.9f,0.3f,0.3f,1.0f },//JointType_ShoulderLeft = 4,
	{ 0.9f,0.3f,0.3f,1.0f },//JointType_ElbowLeft = 5,
	{ 0.9f,0.3f,0.3f,1.0f },//JointType_WristLeft = 6,
	{ 0.9f,0.3f,0.3f,1.0f },//JointType_HandLeft = 7,
	{ 0.3f,0.3f,0.9f,1.0f },//JointType_ShoulderRight = 8,
	{ 0.3f,0.3f,0.9f,1.0f },//JointType_ElbowRight = 9,
	{ 0.3f,0.3f,0.9f,1.0f },//JointType_WristRight = 10,
	{ 0.3f,0.3f,0.9f,1.0f },//JointType_HandRight = 11,
	{ 0.9f,0.9f,0.3f,1.0f },//JointType_HipLeft = 12,
	{ 0.9f,0.9f,0.3f,1.0f },//JointType_KneeLeft = 13,
	{ 0.9f,0.9f,0.3f,1.0f },//JointType_AnkleLeft = 14,
	{ 0.9f,0.9f,0.3f,1.0f },//JointType_FootLeft = 15,
	{ 0.3f,0.9f,0.9f,1.0f },//JointType_HipRight = 16,
	{ 0.3f,0.9f,0.9f,1.0f },//JointType_KneeRight = 17,
	{ 0.3f,0.9f,0.9f,1.0f },//JointType_AnkleRight = 18,
	{ 0.3f,0.9f,0.9f,1.0f },//JointType_FootRight = 19,
	{ 0.0f,0.0f,0.0f,0.0f },//JointType_SpineShoulder = 20,
	{ 0.9f,0.3f,0.3f,1.0f },//JointType_HandTipLeft = 21,
	{ 0.9f,0.3f,0.3f,1.0f },//JointType_ThumbLeft = 22,
	{ 0.3f,0.3f,0.9f,1.0f },//JointType_HandTipRight = 23,
	{ 0.3f,0.3f,0.9f,1.0f },//JointType_ThumbRight = 24,
	//JointType_Count = (JointType_ThumbRight + 1)
};

pair<JointType, JointType> XFeaturePairs[] = {
	{ JointType_SpineBase, JointType_SpineShoulder },
	{ JointType_SpineShoulder, JointType_Head },
	{ JointType_ShoulderLeft, JointType_ElbowLeft },
	{ JointType_ShoulderRight, JointType_ElbowRight },
	{ JointType_ElbowLeft, JointType_HandLeft },
	{ JointType_ElbowRight, JointType_HandRight },
	{ JointType_HipLeft, JointType_KneeLeft },
	{ JointType_HipRight, JointType_KneeRight },
	{ JointType_KneeLeft, JointType_AnkleLeft },
	{ JointType_KneeRight, JointType_AnkleRight },
	//{ JointType_HandLeft, JointType_HandTipLeft },
	//{ JointType_HandRight, JointType_HandTipRight },
	//{ JointType_HandLeft, JointType_ThumbLeft },
	//{ JointType_HandRight, JointType_ThumbRight },
};

JointType KeyJoints[] = {
	JointType_SpineBase,		//1
	JointType_SpineShoulder,	//2
	JointType_Head,				//3
	JointType_ShoulderLeft,		//4
	JointType_ElbowLeft,		//5
	JointType_WristLeft,		//6
	JointType_ShoulderRight,	//7
	JointType_ElbowRight,		//8
	JointType_WristRight,		//9
	JointType_HipLeft,			//10
	JointType_KneeLeft,			//11
	JointType_AnkleLeft,		//12
	JointType_HipRight,			//13
	JointType_KneeRight,		//14
	JointType_AnkleRight,		//15
};

float BoneRadius[JointType_Count] = {
};

#define BEGIN_TO_END(range) range.begin(), range.end()


void SetGlowBoneColor(CharacterGlowParts* glow, const CharacterController& controller);

// Player Proxy methods
void PlayerProxy::StreamPlayerFrame(const TrackedBody& body, const TrackedBody::FrameType& frame)
{
	using namespace Eigen;
	using namespace DirectX;

	m_CurrentPlayerFrame = frame;
	if (g_IngnoreInputRootRotation)
	{
		auto& rotBone = m_CurrentPlayerFrame[m_pPlayerArmature->root()->ID];
		rotBone.GblRotation = rotBone.LclRotation = Quaternion::Identity;
		m_CurrentPlayerFrame.RebuildGlobal(*m_pPlayerArmature);
	}

	bool newMetric = m_CyclicInfo.StreamFrame(m_CurrentPlayerFrame);
	if (newMetric && !m_mapTaskOnGoing)
	{
		m_mapTaskOnGoing = true;
		m_mapTask = concurrency::create_task([this]() {
			auto idx = MapCharacterByLatestMotion();
			m_mapTaskOnGoing = false;

		});
	}

}

void PlayerProxy::ResetPlayer(TrackedBody * pOld, TrackedBody * pNew)
{
	m_CyclicInfo.ResetStream();
	m_CyclicInfo.EnableCyclicMotionDetection(true);
	SetActiveController(-1);
}


PlayerProxy::PlayerProxy()
	: m_IsInitialized(false),
	m_playerSelector(nullptr),
	m_CurrentIdx(-1),
	current_time(0),
	m_mapTaskOnGoing(false),
	m_EnableOverShoulderCam(false),
	m_DefaultCameraFlag(true)
{
	m_pParts = &g_PlayerParts;

	m_pKinect = Devices::KinectSensor::GetForCurrentView();
	m_pPlayerArmature = &m_pKinect->Armature();
	if (g_PlayerParts.empty())
	{
		g_PlayerParts.SetArmature(*m_pPlayerArmature);
	}

	m_CyclicInfo.Initialize(*m_pParts, time_seconds(0.5), time_seconds(3), 30, 0);
	m_CyclicInfo.EnableCyclicMotionDetection(true);

	auto fReset = std::bind(&PlayerProxy::ResetPlayer, this, placeholders::_1, placeholders::_2);
	m_playerSelector.SetPlayerChangeCallback(fReset);

	auto fFrame = std::bind(&PlayerProxy::StreamPlayerFrame, this, placeholders::_1, placeholders::_2);
	m_playerSelector.SetFrameCallback(fFrame);

	m_playerSelector.Initialize(m_pKinect.get(), TrackedBodySelector::SelectionMode::Closest);
	Register();
	m_IsInitialized = true;
}

PlayerProxy::~PlayerProxy()
{
	Unregister();
	//std::ofstream fout("handpos.txt", std::ofstream::out);

	//fout.close();
}

void PlayerProxy::AddChild(SceneObject* pChild)
{
	SceneObject::AddChild(pChild);
	auto pChara = dynamic_cast<CharacterObject*>(pChild);
	if (pChara)
	{
		m_Controllers.emplace_back();
		auto& controller = m_Controllers.back();
		controller.ID = m_Controllers.size() - 1;
		controller.Initialize(*m_pPlayerArmature, *pChara);
		pChara->SetOpticity(1.0f);

		if (g_DebugLocalMotion)
		{
			g_DebugLocalMotionAction[pChara->Name] = pChara->CurrentActionName();
			pChara->StopAction();
		}

		auto glow = pChara->FirstChildOfType<CharacterGlowParts>();
		if (glow == nullptr)
		{
			glow = new CharacterGlowParts();
			glow->SetEnabled(false);
			pChara->AddChild(glow);
		}
	}
}

void SetGlowBoneColorPartPair(Causality::CharacterGlowParts * glow, int Jx, int Jy, const DirectX::XMVECTORF32 *colors, const Causality::ShrinkedArmature & sparts, const Causality::ShrinkedArmature & cparts);

void SetGlowBoneColor(CharacterGlowParts* glow, const CharacterController& controller)
{
	auto pTrans = &controller.Binding();
	auto pCcaTrans = dynamic_cast<const BlockizedCcaArmatureTransform*>(pTrans);
	auto pPartTrans = dynamic_cast<const PartilizedTransformer*>(pTrans);

	auto& cparts = controller.Character().Behavier().ArmatureParts();
	auto& sparts = g_PlayerParts; //! Not Good!!!
	auto& colors = HumanBoneColors;

	//auto& carmature = controller.Character().Armature();
	//for (int i = 0; i <= carmature.size(); i++)
	//{
	//	glow->SetBoneColor(i, DirectX::Colors::Orange.v);
	//}
	//return;

	glow->ResetBoneColor( Math::Colors::Transparent.v);

	if (pCcaTrans)
	{
		for (auto& tp : pCcaTrans->Maps)
		{
			auto Jx = tp.Jx, Jy = tp.Jy;
			SetGlowBoneColorPartPair(glow, Jx, Jy, colors, sparts, cparts);
		}
	}
	else if (pPartTrans)
	{
		for (auto& tp : pPartTrans->ActiveParts)
		{
			auto Jx = tp.SrcIdx, Jy = tp.DstIdx;
			SetGlowBoneColorPartPair(glow, Jx, Jy, colors, sparts, cparts);
		}
		for (auto& tp : pPartTrans->DrivenParts)
		{
			auto Jx = tp.SrcIdx, Jy = tp.DstIdx;
			SetGlowBoneColorPartPair(glow, Jx, Jy, colors, sparts, cparts);
		}
		for (auto& tp : pPartTrans->AccesseryParts)
		{
			auto Jx = tp.SrcIdx, Jy = tp.DstIdx;
			SetGlowBoneColorPartPair(glow, Jx, Jy, colors, sparts, cparts);
		}
	}

}

void SetGlowBoneColorPartPair(Causality::CharacterGlowParts * glow, int Jx, int Jy, const DirectX::XMVECTORF32 *colors, const Causality::ShrinkedArmature & sparts, const Causality::ShrinkedArmature & cparts)
{
	using namespace Math;
	XMVECTOR color;
	if (Jx == NoInputParts)
		color = Colors::Transparent;
	else if (Jx == ActiveAndDrivenParts)
		color = Colors::LightBlue;
	else if (Jx == ActiveParts)
		color = Colors::ForestGreen;
	else if (Jx >= 0)
		color = colors[sparts[Jx]->Joints.front()->ID];

	using namespace DirectX;
	color = XMVectorSetW(color, 0.5f);
	for (auto joint : cparts[Jy]->Joints)
	{
		glow->SetBoneColor(joint->ID, color);
	}
}

void PlayerProxy::SetActiveController(int idx)
{
	std::lock_guard<std::mutex> guard(m_controlMutex);

	if (idx >= 0)
		idx = idx % m_Controllers.size();

	if (idx == -1)
		ResetPrimaryCameraPoseToDefault();

	for (auto& c : m_Controllers)
	{
		if (c.ID != idx)
		{
			auto& chara = c.Character();

			if (!g_DebugLocalMotion && !g_DebugLocalMotionAction[chara.Name].empty())
			{
				auto& action = g_DebugLocalMotionAction[chara.Name];
				chara.StartAction(action);
				g_DebugLocalMotionAction[chara.Name] = "";
			}

			chara.SetOpticity(0.5f);
			auto glow = chara.FirstChildOfType<CharacterGlowParts>();
			glow->SetEnabled(false);

			if (c.ID == m_CurrentIdx && m_CurrentIdx != idx)
			{
				chara.SetPosition(c.CMapRefPos);
				chara.SetOrientation(c.CMapRefRot);
				chara.EnabeAutoDisplacement(false);
			}
		}
	}

	if (m_CurrentIdx != idx)
	{
		m_CurrentIdx = idx;
		if (m_CurrentIdx != -1)
		{
			auto& controller = GetController(m_CurrentIdx);
			auto& chara = controller.Character();

			if (!g_DebugLocalMotion && !chara.CurrentActionName().empty())
			{
				g_DebugLocalMotionAction[chara.Name] = chara.CurrentActionName();
				chara.StopAction();
			}

			chara.SetOpticity(1.0f);

			auto glow = chara.FirstChildOfType<CharacterGlowParts>();
			if (glow)
			{
				glow->SetEnabled(true);
				SetGlowBoneColor(glow, controller);
			}

			controller.MapRefPos = m_playerSelector->PeekFrame()[0].GblTranslation;
			controller.LastPos = controller.MapRefPos;
			controller.CMapRefPos = chara.GetPosition();

			controller.MapRefRot = m_playerSelector->PeekFrame()[0].GblRotation;
			controller.CMapRefRot = chara.GetOrientation();

			chara.EnabeAutoDisplacement(g_UsePersudoPhysicsWalk);
		}
	}
	else
	{
		if (m_CurrentIdx == -1)
			return;

		auto& controller = GetController(m_CurrentIdx);
		auto& chara = controller.Character();

		auto glow = chara.FirstChildOfType<CharacterGlowParts>();
		if (glow)
		{
			SetGlowBoneColor(glow, controller);
			glow->SetEnabled(!g_DebugView);
		}
	}
}

float GetConstraitedRotationFromSinVector(Eigen::Matrix3f &Rot, const Eigen::MatrixXf &covXY, int pivot)
{
	//RowVector3f angles;
	//for (int i = 0; i < 3; i++)
	//{
	//	float sin = sinRot[i];
	//	if (sin < -1.0f || sin > 1.0f)
	//		angles[i] = 0;
	//	else
	//		angles[i] = asinf(sin);
	//}

	//DirectX::Matrix4x4 rot = DirectX::XMMatrixRotationRollPitchYaw(angles[0], angles[1], angles[2]);
	//for (int i = 0; i < 3; i++)
	//	for (int j = 0; j < 3; j++)
	//		Rot(i, j) = rot(i, j);

	//return;

	// Assumption on one axis rotation

	//DenseIndex pivot = -1;
	//sinRot.cwiseAbs().minCoeff(&pivot);
	float tanX = (covXY(1, 2) - covXY(2, 1)) / (covXY(1, 1) + covXY(2, 2));
	float tanY = (covXY(2, 0) - covXY(0, 2)) / (covXY(0, 0) + covXY(2, 2));
	float tanZ = (covXY(0, 1) - covXY(1, 0)) / (covXY(0, 0) + covXY(1, 1));
	//assert(sin <= 1.0f && sin >= -1.0f && "sin value must within range [0,1]");

	// there is nothing bad about using positive value of cosine, it ensure the angle set in [-pi/2,pi/2]
	float cosX = 1.0f / sqrt(1 + tanX*tanX);
	float sinX = cosX * tanX;
	float cosY = 1.0f / sqrt(1 + tanY*tanY);
	float sinY = cosY * tanY;
	float cosZ = 1.0f / sqrt(1 + tanZ*tanZ);
	float sinZ = cosZ * tanZ;

	sinX = -sinX;
	sinY = -sinY;
	sinZ = -sinZ;
	Rot.setIdentity();

	//! IMPORTANT, Right-Hand 
	switch (pivot)
	{
	case 0:
		Rot(1, 1) = cosX;
		Rot(1, 2) = -sinX;
		Rot(2, 2) = cosX;
		Rot(2, 1) = sinX;
		break;
	case 1:
		Rot(0, 0) = cosY;
		Rot(0, 2) = sinY;
		Rot(2, 2) = cosY;
		Rot(2, 0) = -sinY;
		break;
	case 2:
		Rot(0, 0) = cosZ;
		Rot(0, 1) = -sinZ;
		Rot(1, 1) = cosZ;
		Rot(1, 0) = sinZ;
		break;
	}
	return atanf(tanX);
}

Matrix3f FindIsometricTransformXY(const Eigen::MatrixXf& X, const Eigen::MatrixXf& Y)
{
	assert(X.cols() == Y.cols() && X.rows() == Y.rows() && X.cols() == 3 && "input X,Y dimension disagree");

	auto uX = X.colwise().mean().eval();
	auto uY = Y.colwise().mean().eval();

	//sum(Xi1*Yi1,Xi2*Yi2,Xi3*Yi3)
	MatrixXf covXY = X.transpose() * Y;

	// The one axis rotation matrix
	Matrix3f BestRot;
	float BestScale, bestAng;
	int bestPiv = -1;
	float bestErr = numeric_limits<float>::max();

	for (int pivot = 0; pivot < 3; pivot++)
	{
		Matrix3f Rot;
		float scale = 1.0f;
		float ang = GetConstraitedRotationFromSinVector(Rot, covXY, pivot);
		// the isometric scale factor
		scale = ((X * Rot).array()*Y.array()).sum() / X.cwiseAbs2().sum();
		float err = (X * scale * Rot - Y).cwiseAbs2().sum();
		if (err < bestErr)
		{
			bestPiv = pivot;
			BestRot = Rot;
			BestScale = scale;
			bestErr = err;
			bestAng = ang;
		}
	}

	if (bestPiv == -1)
	{
		cout << "[!] Error , Failed to find isometric transform about control handle" << endl;
	}
	else
	{
		static char xyz[] = "XYZ";
		cout << "Isometric transform found : Scale [" << BestScale << "] , Rotation along axis [" << xyz[bestPiv] << "] for " << bestAng / DirectX::XM_PI << "pi , Error = " << bestErr << endl;
	}

	return BestScale * BestRot;
}

void CreateControlBinding(CharacterController & controller, const InputClipInfo& iclip);

void SetIdentity(Causality::PcaCcaMap & map, const Eigen::Index &rank);

float max_cols_assignment(Eigen::MatrixXf & A, Eigen::MatrixXf & Scor, std::vector<ptrdiff_t> &matching);

int PlayerProxy::MapCharacterByLatestMotion()
{
	auto& player = *m_playerSelector;

	CharacterController* pControl = nullptr;
	auto& ifacade = m_CyclicInfo.AqucireFacade();
	for (auto& controller : m_Controllers)			//? <= 5 character
	{
		if (!controller.IsReady)
			continue;
		controller.CreateControlBinding(ifacade);

		if (!pControl || controller.CharacterScore > pControl->CharacterScore)
			pControl = &controller;
	}

	// Disable re-matching when the controller has not request
	m_CyclicInfo.EnableCyclicMotionDetection(false);
	m_CyclicInfo.ReleaseFacade();

	if (!pControl) return -1;

	SetActiveController(pControl->ID);

	return pControl->ID;
}

float max_cols_assignment(Eigen::MatrixXf & A, Eigen::MatrixXf & Scor, std::vector<ptrdiff_t> &matching)
{
	VectorXf XScore(A.rows());
	VectorXi XCount(A.rows());
	XCount.setZero();
	XScore.setZero();
	for (int k = 0; k < A.cols(); k++)
	{
		DenseIndex jx;
		Scor.col(k).maxCoeff(&jx);
		auto score = A(jx, k);
		if (score < g_MatchAccepetanceThreshold) // Reject the match if it's less than a threshold
			matching[k] = -1;
		else
		{
			matching[k] = jx;
			XScore(jx) += score;
			++XCount(jx);
		}
	}
	//XScore.array() /= XCount.array().cast<float>();
	return XScore.sum();
}

void SetIdentity(Causality::PcaCcaMap & map, const Eigen::Index &rank)
{
	map.A.setIdentity(rank, rank);
	map.B.setIdentity(rank, rank);
	map.uX.setZero(rank);
	map.uY.setZero(rank);
	map.uXpca.setZero(rank);
	map.uYpca.setZero(rank);
	map.pcX.setIdentity(rank, rank);
	map.pcY.setIdentity(rank, rank);
	map.useInvB = true;
	map.invB.setIdentity(rank, rank);
}

bool PlayerProxy::IsMapped() const { return m_CurrentIdx >= 0; }

const CharacterController & PlayerProxy::CurrentController() const {
	for (auto& c : m_Controllers)
	{
		if (c.ID == m_CurrentIdx)
			return c;
	}
}

CharacterController & PlayerProxy::CurrentController() {
	for (auto& c : m_Controllers)
	{
		if (c.ID == m_CurrentIdx)
			return c;
	}
}

const CharacterController & PlayerProxy::GetController(int state) const {
	for (auto& c : m_Controllers)
	{
		if (c.ID == state)
			return c;
	}
}

CharacterController & PlayerProxy::GetController(int state)
{
	for (auto& c : m_Controllers)
	{
		if (c.ID == state)
			return c;
	}
}

void PlayerProxy::OnKeyUp(const KeyboardEventArgs & e)
{
	if (e.Key == VK_OEM_PERIOD || e.Key == '.' || e.Key == '>')
	{
		SetActiveController(m_CurrentIdx + 1);
	}
	else if (e.Key == VK_OEM_COMMA || e.Key == ',' || e.Key == '<')
	{
		SetActiveController(m_CurrentIdx - 1);
	}
	else if (e.Key == 'L')
	{
		// this behavier should not change in mapped mode
		if (IsMapped()) return;

		g_DebugLocalMotion = !g_DebugLocalMotion;
		if (g_DebugLocalMotion)
		{
			for (auto& controller : m_Controllers)
			{
				auto& chara = controller.Character();
				g_DebugLocalMotionAction[chara.Name] = chara.CurrentActionName();
				chara.StopAction();
			}
		}
		else
		{
			for (auto& controller : m_Controllers)
			{
				auto& chara = controller.Character();
				auto& action = g_DebugLocalMotionAction[chara.Name];
				if (!action.empty())
					chara.StartAction(action);
				g_DebugLocalMotionAction[chara.Name] = "";
			}
		}
	}
	else if (e.Key == VK_UP || e.Key == VK_DOWN)
	{
		for (auto& controller : m_Controllers)
		{
			auto& chara = controller.Character();
			auto& clips = chara.Behavier().Clips();
			auto& idx = controller.CurrentActionIndex;
			if (e.Key == VK_UP)
				idx = (idx + 1) % clips.size();
			else
				idx = idx == 0 ? clips.size() - 1 : idx - 1;

			if (g_DebugLocalMotion)
				g_DebugLocalMotionAction[chara.Name] = clips[idx].Name;
			else
				chara.StartAction(clips[idx].Name);
		}
	}
	else if (e.Key == 'P')
	{
		g_EnableDependentControl = !g_EnableDependentControl;
		cout << "Enable Dependency Control = " << g_EnableDependentControl << endl;
	}
	else if (e.Key == 'C')
	{
		m_EnableOverShoulderCam = !m_EnableOverShoulderCam;
		//g_UsePersudoPhysicsWalk = m_EnableOverShoulderCam;
		cout << "Over Shoulder Camera Mode = " << m_EnableOverShoulderCam << endl;
		//cout << "Persudo-Physics Walk = " << g_UsePersudoPhysicsWalk << endl;
	}
	else if (e.Key == 'V')
	{
		g_UsePersudoPhysicsWalk = !g_UsePersudoPhysicsWalk;
		cout << "Persudo-Physics Walk = " << g_UsePersudoPhysicsWalk << endl;
		for (auto& controller : m_Controllers)
		{
			controller.Character().EnabeAutoDisplacement(g_UsePersudoPhysicsWalk);
		}
	}
	else if (e.Key == 'M')
	{
		g_MirrowInputX = !g_MirrowInputX;
		cout << "Kinect Input Mirrowing = " << g_MirrowInputX << endl;
		m_pKinect->EnableMirrowing(g_MirrowInputX);
	}
	else if (e.Key == VK_BACK)
	{
		m_CyclicInfo.ResetStream();
		m_DefaultCameraFlag = true;
	}
	else if (e.Key == VK_NUMPAD1)
	{
		g_NoiseInterpolation[0] -= 0.1f;
		cout << "Local Motion Sythesis Jaming = " << g_NoiseInterpolation << endl;
	}
	else if (e.Key == VK_NUMPAD3)
	{
		g_NoiseInterpolation[0] += 0.1f;
		cout << "Local Motion Sythesis Jaming = " << g_NoiseInterpolation << endl;
	}
	else if (e.Key == VK_NUMPAD2)
	{
		g_NoiseInterpolation[0] = 1.0f;
		cout << "Local Motion Sythesis Jaming = " << g_NoiseInterpolation << endl;
	}
	else if (e.Key == VK_NUMPAD4)
	{
		g_NoiseInterpolation[1] -= 0.1f;
		cout << "Local Motion Sythesis Jaming = " << g_NoiseInterpolation << endl;
	}
	else if (e.Key == VK_NUMPAD6)
	{
		g_NoiseInterpolation[1] += 0.1f;
		cout << "Local Motion Sythesis Jaming = " << g_NoiseInterpolation << endl;
	}
	else if (e.Key == VK_NUMPAD5)
	{
		g_NoiseInterpolation[1] = 1.0f;
		cout << "Local Motion Sythesis Jaming = " << g_NoiseInterpolation << endl;
	}
	else if (e.Key == VK_NUMPAD7)
	{
		g_NoiseInterpolation[2] -= 0.1f;
		cout << "Local Motion Sythesis Jaming = " << g_NoiseInterpolation << endl;
	}
	else if (e.Key == VK_NUMPAD9)
	{
		g_NoiseInterpolation[2] += 0.1f;
		cout << "Local Motion Sythesis Jaming = " << g_NoiseInterpolation << endl;
	}
	else if (e.Key == VK_NUMPAD8)
	{
		g_NoiseInterpolation[2] = 1.0f;
		cout << "Local Motion Sythesis Jaming = " << g_NoiseInterpolation << endl;
	}
	else if (e.Key == 'R')
	{
		ResetPrimaryCameraPoseToDefault();

	}
}

void PlayerProxy::OnKeyDown(const KeyboardEventArgs & e)
{
}

RenderFlags Causality::PlayerProxy::GetRenderFlags() const
{
	return RenderFlags::SpecialEffects;
}

void AddNoise(BoneHiracheryFrame& frame, float sigma)
{
	//static std::random_device rd;
	//static std::mt19937 gen(rd());

	//std::normal_distribution<float> nd(1.0f, sigma);

	//for (auto& bone : frame)
	//{
	//	bone.GblTranslation *= 0.95;//nd(gen);
	//}
}

void PlayerProxy::Update(time_seconds const & time_delta)
{
	SceneObject::Update(time_delta);
	using namespace std;
	using namespace Eigen;

	if (!m_IsInitialized)
		return;

	if (g_DebugLocalMotion && !IsMapped())
	{
		current_time += time_delta;
		BoneHiracheryFrame last_frame;
		BoneHiracheryFrame anotherFrame, anotherLastFrame;
		for (auto& controller : m_Controllers)
		{
			if (!controller.IsReady)
				continue;
			auto& chara = controller.Character();
			auto& actionName = g_DebugLocalMotionAction[chara.Name];
			if (actionName.empty())
				continue;
			auto& action = controller.Character().Behavier()[actionName];
			auto& target_frame = controller.Character().MapCurrentFrameForUpdate();
			auto frame = controller.Character().Armature().default_frame();

			target_frame = frame;
			action.GetFrameAt(frame, current_time);
			action.GetFrameAt(last_frame, current_time - time_delta);

			//auto& anotheraction = controller.Character().Behavier()["run"];
			//anotheraction.GetFrameAt(anotherFrame, current_time);
			//anotheraction.GetFrameAt(anotherLastFrame, current_time - time_delta);

			//for (size_t i = 0; i < frame.size(); i++)
			//{
			//	frame[i].GblTranslation = DirectX::XMVectorLerp(frame[i].GblTranslation, anotherFrame[i].GblTranslation, g_NoiseInterpolation);
			//	last_frame[i].GblTranslation = DirectX::XMVectorLerp(last_frame[i].GblTranslation, anotherLastFrame[i].GblTranslation, g_NoiseInterpolation);
			//}

			// Add motion to non-active joints that visualize more about errors for active joints
			//target_frame = frame;
			//AddNoise(frame, .1f);
			controller.SelfBinding().Transform(target_frame, frame, last_frame, time_delta.count());
		}
		return;
	}

	if (!m_IsInitialized || !m_playerSelector)
		return;

	static long long frame_count = 0;

	auto& player = *m_playerSelector;
	if (!player.IsTracked()) return;

	// no new frame is coming
	if (!player.ReadLatestFrame())
		return;
	const auto& frame = player.PeekFrame();
	m_LastPlayerFrame = m_CurrentPlayerFrame;
	m_CurrentPlayerFrame = frame;

	g_RevampLikilyhoodThreshold = 0.5;
	g_RevampLikilyhoodTimeThreshold = 1.0;
	if (g_ForceRemappingAlwaysOn)
		m_CyclicInfo.EnableCyclicMotionDetection();

	if (IsMapped())
	{
		auto& controller = CurrentController();
		float lik = controller.UpdateTargetCharacter(frame, m_LastPlayerFrame, time_delta.count());

		// Check if we need to "Revamp" Control Binding
		if (lik < g_RevampLikilyhoodThreshold)
		{
			m_LowLikilyTime += time_delta.count();
			if (m_LowLikilyTime > g_RevampLikilyhoodTimeThreshold)
			{
				m_CyclicInfo.EnableCyclicMotionDetection();
			}
		}
		else
		{
			m_CyclicInfo.EnableCyclicMotionDetection(false);
			m_LowLikilyTime = 0;
		}

		if (m_EnableOverShoulderCam)
			UpdatePrimaryCameraForTrack();
		return;
	}
}

void PlayerProxy::UpdatePrimaryCameraForTrack()
{

	auto& camera = *this->Scene->PrimaryCamera();
	auto& cameraPos = dynamic_cast<SceneObject&>(camera);
	auto& contrl = this->CurrentController();
	auto& chara = contrl.Character();
	using namespace DirectX;
	XMVECTOR ext = XMLoad(chara.RenderModel()->GetBoundingBox().Extents);
	ext = XMVector3LengthEst(ext);
	ext *= chara.GetGlobalTransform().Scale;

	if (m_DefaultCameraFlag)
	{
		m_DefaultCameraFlag = false;
		m_DefaultCameraPose.Translation = cameraPos.GetPosition();
		m_DefaultCameraPose.Rotation = cameraPos.GetOrientation();
	}

	cameraPos.SetPosition((XMVECTOR)chara.GetPosition() + XMVector3Rotate(XMVectorMultiplyAdd(ext, XMVectorSet(-2.0f, 2.0f, -2.0f, 0.0f), XMVectorSet(-0.5f, 0.5, -0.5, 0)), chara.GetOrientation()));
	camera.GetView()->FocusAt((XMVECTOR)chara.GetPosition() + XMVector3Rotate(XMVectorMultiplyAdd(ext, XMVectorSet(-2.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(-0.5f, 0.5, -0.5, 0)), chara.GetOrientation()), g_XMIdentityR1.v);
}

void Causality::PlayerProxy::ResetPrimaryCameraPoseToDefault()
{
	// Camera pose not changed by Over Shoulder view
	if (m_DefaultCameraFlag)
		return;

	auto& camera = *this->Scene->PrimaryCamera();
	auto& cameraPos = dynamic_cast<SceneObject&>(camera);

	m_DefaultCameraFlag = true;
	cameraPos.SetPosition(m_DefaultCameraPose.Translation);
	cameraPos.SetOrientation(m_DefaultCameraPose.Rotation);
}

bool PlayerProxy::IsVisible(const DirectX::BoundingGeometry & viewFrustum) const
{
	return true;
}

void DrawJammedGuidingVectors(const ShrinkedArmature & barmature, const BoneHiracheryFrame & frame, const Color & color, const Matrix4x4 & world, float thinkness = 0.015f)
{
	using DirectX::Visualizers::g_PrimitiveDrawer;
	using namespace DirectX;
	if (frame.empty())
		return;
	//g_PrimitiveDrawer.SetWorld(world);
	g_PrimitiveDrawer.SetWorld(XMMatrixIdentity());
	//g_PrimitiveDrawer.Begin();
	for (auto& block : barmature)
	{
		if (block->parent() != nullptr)
		{
			auto& bone = frame[block->Joints.back()->ID];
			XMVECTOR ep = bone.GblTranslation;

			auto& pbone = frame[block->parent()->Joints.back()->ID];
			XMVECTOR sp = pbone.GblTranslation;

			sp = XMVector3Transform(sp, world);
			ep = XMVector3Transform(ep, world);
			//g_PrimitiveDrawer.DrawLine(sp, ep, color);

			//XMVECTOR v = ep - sp;
			//RowVectorXf ux = block->PdGpr.uX.cast<float>();


			g_PrimitiveDrawer.DrawCylinder(sp, ep, g_DebugArmatureThinkness, color);
			g_PrimitiveDrawer.DrawSphere(ep, g_DebugArmatureThinkness * 1.5f, color);
		}
	}
	//g_PrimitiveDrawer.End();


}

void DrawGuidingVectors(const ShrinkedArmature & barmature, const BoneHiracheryFrame & frame, const Color & color, const Matrix4x4 & world, float thinkness = 0.015f)
{
	using DirectX::Visualizers::g_PrimitiveDrawer;
	using namespace DirectX;
	if (frame.empty())
		return;
	//g_PrimitiveDrawer.SetWorld(world);
	g_PrimitiveDrawer.SetWorld(XMMatrixIdentity());
	//g_PrimitiveDrawer.Begin();
	for (auto& block : barmature)
	{
		if (block->parent() != nullptr)
		{
			auto& bone = frame[block->Joints.back()->ID];
			XMVECTOR ep = bone.GblTranslation;

			auto& pbone = frame[block->parent()->Joints.back()->ID];
			XMVECTOR sp = pbone.GblTranslation;

			sp = XMVector3Transform(sp, world);
			ep = XMVector3Transform(ep, world);
			//g_PrimitiveDrawer.DrawLine(sp, ep, color);

			g_PrimitiveDrawer.DrawCylinder(sp, ep, g_DebugArmatureThinkness, color);
			g_PrimitiveDrawer.DrawSphere(ep, g_DebugArmatureThinkness * 1.5f, color);
		}
	}
	//g_PrimitiveDrawer.End();


}

void DrawControllerHandle(const CharacterController& controller)
{
	using DirectX::Visualizers::g_PrimitiveDrawer;
	using namespace Math;

	XMVECTOR color = Colors::Pink;
	XMVECTOR vel_color = Colors::Navy;


	g_PrimitiveDrawer.SetWorld(XMMatrixIdentity());
	XMMATRIX world = controller.Character().GlobalTransformMatrix();

	auto& barmature = controller.Character().Behavier().ArmatureParts();
	auto& frame = controller.Character().GetCurrentFrame();

	for (auto& block : barmature)
	{
		if (block->ActiveActions.size() > 0)
		{
			auto& handle = controller.PvHandles()[block->Index];
			XMVECTOR ep = handle.first;

			auto& pbone = frame[block->parent()->Joints.back()->ID];
			XMVECTOR sp = pbone.GblTranslation;
			ep = sp + ep;

			sp = XMVector3Transform(sp, world);
			ep = XMVector3Transform(ep, world);

			g_PrimitiveDrawer.DrawCylinder(sp, ep, g_DebugArmatureThinkness, color);
			g_PrimitiveDrawer.DrawSphere(ep, g_DebugArmatureThinkness * 1.5f, color);
			sp = ep;
			ep = handle.second;
			ep = XMVector3TransformNormal(ep, world);
			ep = sp + ep;
			g_PrimitiveDrawer.DrawCylinder(sp, ep, g_DebugArmatureThinkness, vel_color);
			g_PrimitiveDrawer.DrawCone(ep, ep - sp, g_DebugArmatureThinkness * 5, g_DebugArmatureThinkness * 3, vel_color);
		}
	}
}

void PlayerProxy::Render(IRenderContext * context, DirectX::IEffect* pEffect)
{
	BoneHiracheryFrame charaFrame;

	if (g_DebugLocalMotion && g_DebugView)
	{
		for (auto& controller : m_Controllers)
		{
			if (!controller.IsReady)
				continue;
			auto& chara = controller.Character();
			auto& action = controller.Character().Behavier()[g_DebugLocalMotionAction[chara.Name]];
			action.GetFrameAt(charaFrame, current_time);
			auto world = chara.GlobalTransformMatrix();
			DrawArmature(chara.Armature(), charaFrame, DirectX::Colors::LimeGreen.v, world, g_DebugArmatureThinkness / chara.GetGlobalTransform().Scale.x);
			DrawControllerHandle(controller);
		}
	}

	if (!m_playerSelector) return;
	auto& player = *m_playerSelector;

	Color color = DirectX::Colors::Yellow.v;

	if (player.IsTracked())
	{
		const auto& frame = player.PeekFrame();

		if (IsMapped())
			color.A(0.3f);

		DrawArmature(*player.BodyArmature, frame, reinterpret_cast<const Color*>(HumanBoneColors));
	}

	// IsMapped() && 
	if (IsMapped() && g_DebugView)
	{
		//auto& controller = this->CurrentController().Character();
		for (auto& controller : m_Controllers)
		{
			if (!controller.IsReady)
				continue;

			auto& chara = controller.Character();
			DrawControllerHandle(controller);
		}

	}

}

void XM_CALLCONV PlayerProxy::UpdateViewMatrix(DirectX::FXMMATRIX view, DirectX::CXMMATRIX projection)
{
	DirectX::Visualizers::g_PrimitiveDrawer.SetView(view);
	DirectX::Visualizers::g_PrimitiveDrawer.SetProjection(projection);
}

KinectVisualizer::KinectVisualizer()
{
	pKinect = Devices::KinectSensor::GetForCurrentView();
}

bool KinectVisualizer::IsVisible(const DirectX::BoundingGeometry & viewFrustum) const
{
	return true;
}

void KinectVisualizer::Render(IRenderContext * context, DirectX::IEffect* pEffect)
{
	auto &players = pKinect->GetTrackedBodies();
	using DirectX::Visualizers::g_PrimitiveDrawer;

	for (auto& player : players)
	{
		if (player.IsTracked())
		{
			const auto& frame = player.PeekFrame();

			DrawArmature(*player.BodyArmature, frame, DirectX::Colors::LimeGreen.v);
		}
	}
}

void XM_CALLCONV KinectVisualizer::UpdateViewMatrix(DirectX::FXMMATRIX view, DirectX::CXMMATRIX projection)
{
	DirectX::Visualizers::g_PrimitiveDrawer.SetView(view);
	DirectX::Visualizers::g_PrimitiveDrawer.SetProjection(projection);
}

RenderFlags Causality::KinectVisualizer::GetRenderFlags() const
{
	return RenderFlags::SpecialEffects;
}

