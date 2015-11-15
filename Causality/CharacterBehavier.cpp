#include "pch_bcl.h"
#include "CharacterBehavier.h"

using namespace Causality;
using namespace DirectX;
using namespace Eigen;
using namespace std;


Eigen::VectorXf BehavierSpace::FrameFeatureVectorLnQuaternion(const frame_type & frame) const
{
	Eigen::VectorXf fvector(frame.size() * 3);
	Eigen::Vector3f v{ 0,1,0 };

	for (size_t i = 0; i < frame.size(); i++)
	{
		const auto& bone = frame[i];
		XMVECTOR q = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&bone.LclRotation));
		DirectX::Quaternion lq = XMQuaternionLn(bone.LclRotation);
		Eigen::Map<const Eigen::Vector3f> elq(&lq.x);
		fvector.block<3, 1>(i * 3, 0) = elq;
	}

	fvector = fvector.cwiseProduct(Wb);
	return fvector;
}

BehavierSpace::animation_type & BehavierSpace::operator[](const std::string & name)
{
	for (auto& clip : m_AnimClips)
	{
		if (clip.Name == name) return clip;
	}
	throw std::out_of_range("No animation with specified name exist in this aniamtion space.");
}

const BehavierSpace::animation_type & BehavierSpace::operator[](const std::string & name) const
{
	for (const auto& clip : m_AnimClips)
	{
		if (clip.Name == name) return clip;
	}
	throw std::out_of_range("No animation with specified name exist in this aniamtion space.");
}

void Causality::BehavierSpace::AddAnimationClip(animation_type && animation)
{
#ifdef _DEBUG
	for (auto& clip : m_AnimClips)
		if (clip.Name == animation.Name)
		throw std::out_of_range("Name of animation is already existed.");
#endif
	m_AnimClips.emplace_back(std::move(animation));
}

BehavierSpace::animation_type & BehavierSpace::AddAnimationClip(const std::string & name)
{
#ifdef _DEBUG
	for (auto& clip : m_AnimClips)
		if (clip.Name == name)
		throw std::out_of_range("Name of animation is already existed.");
#endif
	m_AnimClips.emplace_back(name);
	auto& anim = m_AnimClips.back();
	anim.SetArmature(*m_pArmature);
	anim.SetDefaultFrame(m_pArmature->default_frame());
	return anim;
}

bool Causality::BehavierSpace::Contains(const std::string & name) const
{
	for (auto& clip : m_AnimClips)
	{
		if (clip.Name == name) return true;
	}
	return false;
}

void Causality::BehavierSpace::UpdateArmatureParts()
{
	auto& armature = *m_pArmature;
	m_Parts.SetArmature(armature);
}

const IArmature & BehavierSpace::Armature() const { return *m_pArmature; }

IArmature & BehavierSpace::Armature() { return *m_pArmature; }

void BehavierSpace::SetArmature(IArmature & armature) {
	assert(this->Clips().empty());
	m_pArmature = &armature;
	UpdateArmatureParts();
}

const BehavierSpace::frame_type & BehavierSpace::RestFrame() const { return Armature().default_frame(); }

void BehavierSpace::UniformQuaternionsBetweenClips()
{
	auto& armature = *m_pArmature;
	auto& clips = m_AnimClips;

	// Try to unify rotation pivot direction
	typedef vector<Vector4, DirectX::AlignedAllocator<Vector4, alignof(DirectX::XMVECTOR)>> aligned_vector_of_vector4;
	aligned_vector_of_vector4 gbl_pivots(armature.size());
	bool gbl_def = true;
	for (auto& anim : clips)
	{
		using namespace DirectX;
		aligned_vector_of_vector4 pivots(armature.size());
		for (auto& frame : anim.GetFrameBuffer())
		{
			for (size_t i = 0; i < armature.size(); i++)
			{
				pivots[i] += frame[i].LclRotation.LoadA();
			}
		}

		if (gbl_def)
		{
			for (size_t i = 0; i < armature.size(); i++)
			{
				gbl_pivots[i] = DirectX::XMVector3Normalize(pivots[i].LoadA());
			}
			gbl_def = false;
		}
		else
		{
			//for (size_t i = 0; i < armature.size(); i++)
			//{
			//	XMVECTOR pivot = DirectX::XMVector3Normalize(pivots[i].LoadA());
			//	XMVECTOR gbl_pivot = gbl_pivots[i].Load();
			//	if (XMVector4Less(XMVector3Dot(gbl_pivot, pivot), XMVectorZero()))
			//	{
			//		for (auto& frame : anim.GetFrameBuffer())
			//		{
			//			frame[i].LclRotation.StoreA(-frame[i].LclRotation.LoadA());
			//		}
			//	}
			//}
		}
	}
}

Eigen::VectorXf BehavierSpace::FrameFeatureVectorEndPointNormalized(const frame_type & frame) const
{
	int N = frame.size();
	Eigen::VectorXf fvector(N * 3);
	//Eigen::Vector3f v{ 0,1,0 };
	XMVECTOR v = g_XMIdentityR1.v;
	std::vector<Vector3> sv(N + 1); // Allocate one more space
	for (int i = 0; i < N; i++)
	{
		const auto& bone = frame[i];
		XMVECTOR q = bone.GblTranslation;
		q = XMVector3Rotate(v, q);
		XMStoreFloat4(&reinterpret_cast<XMFLOAT4&>(sv[i]), q);
		// HACK: equals to sv[i]=q
		// Float4 functions is significant faster
		// and all memery address is accessable = =
	}

	using namespace Eigen;
	Map<const VectorXf> eep(&sv[0].x, N * 3);
	//Map<const Matrix3Xf, Aligned, Stride<1, sizeof(Bone) / sizeof(float)>> eep(&frame[0].GblTranslation.x, N);

	//Map<const Matrix3Xf,Aligned, Stride<1,sizeof(Bone)/sizeof(float)>> eep(&frame[0].GblTranslation.x, N);
	fvector = eep.cwiseProduct(Wb);//.asDiagonal();
	return fvector;
}

Eigen::MatrixXf Causality::BehavierSpace::AnimationMatrixEndPosition(const ArmatureFrameAnimation & animation) const
{
	const auto & frames = animation.GetFrameBuffer();
	int K = animation.GetFrameBuffer().size();
	int N = animation.Armature().size();
	Eigen::MatrixXf fmatrix(N * 3, K);
	CacAnimationMatrixEndPosition(animation, fmatrix);
	return fmatrix;
}

void Causality::BehavierSpace::CacAnimationMatrixEndPosition(const ArmatureFrameAnimation & animation, Eigen::MatrixXf & fmatrix) const
{
	const auto & frames = animation.GetFrameBuffer();
	int K = animation.GetFrameBuffer().size();
	int N = animation.Armature().size();
	if (fmatrix.rows() != N * 3 || fmatrix.cols() != K)
		fmatrix.resize(N * 3, K);
	for (size_t i = 0; i < K; i++)
	{
		auto& frame = frames[i];
		auto fvector = fmatrix.col(i);
		//Eigen::Vector3f v{ 0,1,0 };
		XMVECTOR v = g_XMIdentityR1.v;
		std::vector<Vector3> sv(N + 1); // Allocate one more space
		for (int i = 0; i < N; i++)
		{
			const auto& bone = frame[i];
			XMVECTOR q = bone.GblTranslation;
			q = XMVector3Rotate(v, q);
			XMStoreFloat4(&reinterpret_cast<XMFLOAT4&>(sv[i]), q);
			// HACK: equals to sv[i]=q
			// Float4 functions is significant faster
			// and all memery address is accessable = =
		}

		using namespace Eigen;
		Map<const VectorXf> eep(&sv[0].x, N * 3);
		//Map<const Matrix3Xf, Aligned, Stride<1, sizeof(Bone) / sizeof(float)>> eep(&frame[0].GblTranslation.x, N);

		//Map<const Matrix3Xf,Aligned, Stride<1,sizeof(Bone)/sizeof(float)>> eep(&frame[0].GblTranslation.x, N);
		fvector = eep.cwiseProduct(Wb);//.asDiagonal();
	}
}

// !!! This will NOT work since the space will always be full-rank (with enough key frames)
// and the projection will always be the same as the input feature vector
// How about doing a PCA?
float BehavierSpace::PoseDistancePCAProjection(const frame_type & frame) const
{
	auto fv = FrameFeatureVectorLnQuaternion(frame);

	// Assupt the space have mutiple basis
	fv -= X0;
	auto w = XpInv * fv;
	auto w0 = 1.0f - w.sum();

	VectorXf px = X * w;
	px += w0 * X0;

	// px is the projected point from fv to Space [X0 X]
	auto distance = (fv - px).norm();

	return distance;
}

Eigen::RowVectorXf BehavierSpace::PoseSquareDistanceNearestNeibor(const frame_type & frame) const
{
	auto fv = FrameFeatureVectorLnQuaternion(frame);

	// Here X is an dense sampled frames feature vector from the animations
	auto D = X - fv.replicate(1, X.cols());
	auto dis = D.colwise().squaredNorm();
	return dis;
}

Eigen::RowVectorXf Causality::BehavierSpace::StaticSimiliartyEculidDistance(const frame_type & frame) const
{
	using namespace Eigen;
	auto dis = PoseSquareDistanceNearestNeibor(frame);
	dis /= -(SegmaDis*SegmaDis);
	dis = dis.array().exp();
	return dis;
}

Eigen::VectorXf BehavierSpace::CaculateFrameDynamicFeatureVectorJointVelocityHistogram(const frame_type & frame, const frame_type & prev_frame) const
{
	using namespace Eigen;
	auto N = frame.size();
	VectorXf Jvh(N);
	for (size_t i = 0; i < N; i++)
	{
		auto disp = frame[i].GblTranslation - prev_frame[i].GblTranslation;
		Jvh(i) = disp.Length();
	}

	Jvh.normalize();
	return Jvh;
}

// Nothing but Bhattacharyya distance
Eigen::RowVectorXf BehavierSpace::DynamicSimiliarityJvh(const frame_type & frame, const frame_type & prev_frame) const
{
	auto fv = CaculateFrameDynamicFeatureVectorJointVelocityHistogram(frame, prev_frame);
	Eigen::MatrixXf Dis = Xv.cwiseProduct(fv.replicate(1, Xv.cols()));
	Dis = Dis.cwiseSqrt();
	return Dis.colwise().sum();;
}

//float BehavierSpace::DynamicSimiliarityJvh(const velocity_frame_type & velocity_frame) const
//{
//	
//}

float Causality::BehavierSpace::FrameLikilihood(const frame_type & frame, const frame_type & prev_frame) const
{
	auto stsim = StaticSimiliartyEculidDistance(frame);
	auto dysim = DynamicSimiliarityJvh(frame, prev_frame);
	stsim = stsim.cwiseProduct(dysim);
	return stsim.minCoeff();
}

float Causality::BehavierSpace::FrameLikilihood(const frame_type & frame) const
{
	auto minDis = PoseSquareDistanceNearestNeibor(frame).minCoeff();
	return expf(-minDis / (SegmaDis*SegmaDis));
}

vector<ArmatureTransform> BehavierSpace::GenerateBindings()
{
	vector<ArmatureTransform> bindings;
	auto& armature = Armature();

	for (auto& joint : armature.joints())
	{
		joint.AssignSemanticsBasedOnName();
	}
	return bindings;
}

void BehavierSpace::CaculateXpInv()
{
	auto Xt = X.transpose();
	XpInv = Xt * X;
	XpInv.ldlt().solveInPlace(Xt);
}
