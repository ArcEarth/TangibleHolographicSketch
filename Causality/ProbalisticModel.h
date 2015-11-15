#pragma once
#include "Common\Locatable.h"

namespace Causality
{
	struct ProbalisticBase
	{
	public:
		float Probability;
		float GetProbality() const { return Probability; }
		void SetProbality(float value) { Probability = value; }
	};

	struct ProbalisticRigidState : public DirectX::RigidBase, public ProbalisticBase
	{
	};

	struct ProbalisticRigidObject
	{
	public:
		std::vector<ProbalisticRigidState> States;
		DirectX::RigidBase MeanState() const;
		const ProbalisticRigidState &MostLikelyState() const;
	};
}
