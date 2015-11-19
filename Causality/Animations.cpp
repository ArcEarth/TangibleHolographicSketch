#include "pch_bcl.h"
#include "Animations.h"
#include "BoneFeatures.h"

using namespace Causality;
using namespace DirectX;
using namespace Eigen;
using namespace std;

size_t Causality::CLIP_FRAME_COUNT = 90U;

BoneHiracheryFrame::BoneHiracheryFrame(size_t size)
	: BaseType(size)
{
}

BoneHiracheryFrame::BoneHiracheryFrame(const IArmature & armature)
	: BaseType(armature.default_frame())
{
}

void BoneHiracheryFrame::RebuildGlobal(const IArmature & armature)
{
	for (auto& joint : armature.joints())
	{
		auto& bone = at(joint.ID);
		if (joint.is_root())
		{
			bone.GblRotation = bone.LclRotation;
			bone.GblScaling = bone.LclScaling;
			bone.GblTranslation = bone.LclTranslation;
			bone.LclLength = bone.GblLength = 1.0f; // Length of root doesnot have any meaning
		}
		else
		{
			//bone.OriginPosition = at(joint.ParentID).GblTranslation;

			bone.UpdateGlobalData(at(joint.ParentID));
		}
	}
}

void BoneHiracheryFrame::RebuildLocal(const IArmature & armature)
{
	for (auto& joint : armature.joints())
	{
		auto& bone = at(joint.ID);
		if (joint.is_root())
		{
			bone.LclRotation = bone.GblRotation;
			bone.LclScaling = bone.GblScaling;
			bone.LclTranslation = bone.GblTranslation;
			bone.LclLength = bone.GblLength = 1.0f; // Length of root doesnot have any meaning
		}
		else
		{
			bone.UpdateLocalData(at(joint.ParentID));
		}
	}
}

//Eigen::VectorXf BoneHiracheryFrame::LocalRotationVector() const
//{
//	Eigen::VectorXf fvector(size() * 3);
//	Eigen::Vector3f v{ 0,1,0 };
//
//	for (size_t i = 0; i < size(); i++)
//	{
//		const auto& bone = (*this)[i];
//		XMVECTOR q = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&bone.LclRotation));
//		DirectX::Quaternion lq = XMQuaternionLn(bone.LclRotation);
//		Eigen::Map<const Eigen::Vector3f> elq(&lq.x);
//		fvector.block<3, 1>(i * 3, 0) = elq;
//	}
//
//	return fvector;
//}
//
//void BoneHiracheryFrame::UpdateFromLocalRotationVector(const IArmature& armature, const Eigen::VectorXf fv)
//{
//	auto& This = *this;
//	auto& sarmature = static_cast<const StaticArmature&>(armature);
//	for (auto i : sarmature.joint_indices())
//	{
//		auto data = fv.block<3, 1>(i * 3, 0).data();
//		This[i].LclRotation = XMQuaternionExp(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(data)));
//		if (!armature[i]->is_root())
//		{
//			This[i].UpdateGlobalData(This[armature[i]->ParentID]);
//		}
//	}
//}

void BoneHiracheryFrame::Lerp(BoneHiracheryFrame& out, const BoneHiracheryFrame & lhs, const BoneHiracheryFrame & rhs, float t, const IArmature& armature)
{
	//assert((Armature == lhs.pArmature) && (lhs.pArmature == rhs.pArmature));
	XMVECTOR vt = XMVectorReplicate(t);
	for (size_t i = 0; i < armature.size(); i++)
	{
		XMStoreA(out[i].LclRotation,DirectX::XMQuaternionSlerpV(XMLoadA(lhs[i].LclRotation), XMLoadA(rhs[i].LclRotation), vt));
		XMStoreA(out[i].LclScaling,DirectX::XMVectorLerpV(XMLoadA(lhs[i].LclScaling), XMLoadA(rhs[i].LclScaling), vt));
		XMStoreA(out[i].LclTranslation,DirectX::XMVectorLerpV(XMLoadA(lhs[i].LclTranslation), XMLoadA(rhs[i].LclTranslation), vt));
	}
	out.RebuildGlobal(armature);
}

void Causality::BoneHiracheryFrame::Difference(BoneHiracheryFrame & out, const BoneHiracheryFrame & from, const BoneHiracheryFrame & to)
{
	auto n = std::min(from.size(), to.size());
	if (out.size() < n)
		out.resize(n);
	for (size_t i = 0; i < n; i++)
	{
		auto& lt = out[i];
		lt.LocalTransform() = from[i].LocalTransform();
		lt.LocalTransform().Inverse();
		lt.LocalTransform() *= to[i].LocalTransform();

		lt.GlobalTransform() = from[i].GlobalTransform();
		lt.GlobalTransform().Inverse();
		lt.GlobalTransform() *= to[i].GlobalTransform();
	}
}

void Causality::BoneHiracheryFrame::Deform(BoneHiracheryFrame & out, const BoneHiracheryFrame & from, const BoneHiracheryFrame & deformation)
{
	auto n = std::min(from.size(), deformation.size());
	if (out.size() < n)
		out.resize(n);
	for (size_t i = 0; i < n; i++)
	{
		auto& lt = out[i];
		lt.LocalTransform() = from[i].LocalTransform();
		lt.LocalTransform() *= deformation[i].LocalTransform();
	}
}

void BoneHiracheryFrame::Blend(BoneHiracheryFrame& out, const BoneHiracheryFrame & lhs, const BoneHiracheryFrame & rhs, float * blend_weights, const IArmature& armature)
{

}

void BoneHiracheryFrame::TransformMatrix(DirectX::XMFLOAT3X4 * pOut, const self_type & from, const self_type & to, size_t numOut)
{
	using namespace std;
	auto n = min(from.size(), to.size());
	if (numOut > 0)
		n = min(n, numOut);
	for (int i = 0; i < n; ++i)
	{
		XMMATRIX mat = Bone::TransformMatrix(from[i], to[i]);
		mat = XMMatrixTranspose(mat);
		XMStoreFloat3x4(pOut + i, mat);
	}
}

void BoneHiracheryFrame::TransformMatrix(DirectX::XMFLOAT4X4 * pOut, const self_type & from, const self_type & to, size_t numOut)
{
	using namespace std;
	auto n = min(from.size(), to.size());
	if (numOut > 0)
		n = min(n, numOut);
	for (int i = 0; i < n; ++i)
	{
		XMMATRIX mat = Bone::TransformMatrix(from[i], to[i]);
		//mat = XMMatrixTranspose(mat);
		XMStoreFloat4x4(pOut + i, mat);
	}
}


ArmatureFrameAnimation::ArmatureFrameAnimation(std::istream & file)
{
	using namespace std;
	using namespace DirectX;
	self_type animation;
	int keyframs, joints;
	file >> joints >> keyframs;
	animation.KeyFrames.resize(keyframs);
	auto itrKey = KeyFrames.begin();
	for (int i = 0; i < keyframs; i++)
	{
		auto& key = (*itrKey++);
		key.Frame.resize(joints);
		double seconds;
		file >> key.Name;
		file >> seconds;
		key.Time = (time_seconds)seconds;
		for (auto& bone : key.Frame)
		{
			Vector3 vec;
			auto& scl = bone.LclScaling;
			//char ch;
			file >> vec.x >> vec.y >> vec.z >> scl.y;
			bone.LclRotation = XMQuaternionRotationRollPitchYawFromVector(vec);
		}
	}
}

ArmatureFrameAnimation::ArmatureFrameAnimation(const std::string & name)
	: base_type(name)
{
}
//
//ArmatureFrameAnimation::self_type& ArmatureFrameAnimation::operator=(const self_type&& rhs)
//{
//	pArmature = rhs.pArmature;
//	frames = std::move(rhs.frames);
//	QrYs = std::move(rhs.QrYs);
//	Ecj = std::move(rhs.Ecj);
//	base_type::operator=(std::move(rhs));
//	return *this;
//}

bool ArmatureFrameAnimation::InterpolateFrames(double frameRate)
{
	float delta = (float)(1.0 / frameRate);
	auto& armature = *pArmature;
	float t = 0;

	auto itrKey = KeyFrames.begin();
	auto itrKeyAdv = ++KeyFrames.begin();

	for (size_t i = 1; i < KeyFrames.size(); i++)
	{
		const frame_type& lhs = itrKey->Frame;
		const frame_type& rhs = itrKeyAdv->Frame;
		for (t = (float)itrKey->Time.count(); t < (float)itrKey->Time.count(); t += delta)
		{
			frames.emplace_back(armature.size());
			frame_type::Lerp(frames.back(), lhs, rhs, (float)t, armature);
		}
		++itrKey;
		++itrKeyAdv;
	}
	frames.shrink_to_fit();
	return true;
}

bool ArmatureFrameAnimation::GetFrameAt(BoneHiracheryFrame & outFrame, TimeScalarType time) const
{

	double t = fmod(time.count(), Duration.count());
	if (t < 0) t += Duration.count();
	int frameIdx = (int)floorf(t / FrameInterval.count());
	frameIdx = frameIdx % frames.size(); // ensure the index is none negative
	auto& sframe = frames[frameIdx];
	auto& tframe = frames[(frameIdx + 1) % frames.size()];
	t -= frameIdx * FrameInterval.count();
	t /= FrameInterval.count();

	if (outFrame.size() < Armature().size())
		outFrame.resize(Armature().size());
	BoneHiracheryFrame::Lerp(outFrame, sframe, tframe,t, Armature());
	return true;
}

void ArmatureFrameAnimation::Serialize(std::ostream & binary) const
{
	binary << (uint32_t)bonesCount << (uint32_t)frames.size() << (uint32_t)sizeof(Bone);
	binary << (double)Duration.count() << (double)FrameInterval.count();
	for (auto& frame : frames)
	{
		binary.write(reinterpret_cast<const char*>(frame.data()),
			sizeof(Bone)*bonesCount);
	}
}

void ArmatureFrameAnimation::Deserialize(std::istream & binary)
{
	uint32_t bC, fC, sB;
	double dur, itv;
	binary >> bC >> fC >> sB >> dur >> itv;

	Duration = time_seconds(dur);
	FrameInterval = time_seconds(itv);

	assert((int)(dur / itv) == fC);
	assert(sB == sizeof(Bone));

	time_seconds time(0);
	frames.resize(fC);
	for (auto& frame : frames)
	{
		time += FrameInterval;

		frame.resize(bC);
		binary.read(reinterpret_cast<char*>(frame.data()),
			sizeof(Bone)*bonesCount);
	}
}

ArmatureTransform::ArmatureTransform() {
	pSource = nullptr;
	pTarget = nullptr;
}

void ArmatureTransform::TransformBack(frame_type & source_frame, const frame_type & target_frame) const
{
}

void Causality::ScaleFrame(BoneHiracheryFrame & frame, const BoneHiracheryFrame & ref, float scale)
{
	XMVECTOR sv = XMVectorReplicate(scale);
	for (size_t i = 0; i < frame.size(); i++)
	{
		auto& lt = frame[i].LocalTransform();
		auto& rt = ref[i].LocalTransform();
		IsometricTransform::LerpV(lt, rt, lt, sv);
	}
}
