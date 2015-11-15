SETTING_REGISTERATION(int, PartAssignmentTransform, PAT_RST)
SETTING_REGISTERATION(bool, UsePersudoPhysicsWalk, false)
SETTING_REGISTERATION(int, PhaseMatchingInterval, 1)

SETTING_REGISTERATION(float, MaxCharacterSpeed, 0.5)
SETTING_REGISTERATION(bool, EnableDebugLogging, true)
SETTING_REGISTERATION(bool, EnableRecordLogging, false)

SETTING_REGISTERATION(bool, StepFrame, false)

//	When this flag set to true, subtle motion will be synthesised by major motion
SETTING_REGISTERATION(bool, EnableDependentControl, false)

//	When this flag set to true, the kinect input's rotation will be ingnored
SETTING_REGISTERATION(bool, IngnoreInputRootRotation, true)

SETTING_REGISTERATION(float, FrameTimeScaleFactor, 20)

SETTING_REGISTERATION(float, PlayerPcaCutoff, 0.04f) // 0.2^2
//static const float	EnergyCutoff = 0.3f;

SETTING_REGISTERATION(float, PlayerActiveEnergy, 0.3f)
SETTING_REGISTERATION(float, PlayerSubactiveEnergy, 0.03f)

SETTING_REGISTERATION(float, BlendWeight, 0.8f)

SETTING_REGISTERATION(float, AssignWeight, 1.0f)

SETTING_REGISTERATION(float, MatchAccepetanceThreshold, 0.2f)

SETTING_REGISTERATION(bool, EnableInputFeatureLocalization, true)

SETTING_REGISTERATION(bool, LoadCharacterModelParameter, true)
SETTING_REGISTERATION(float, CharacterPcaCutoff, 0.004f) // 0.2^2
SETTING_REGISTERATION(float, CharacterActiveEnergy, 0.40f)
SETTING_REGISTERATION(float, CharacterSubactiveEnergy, 0.02f)

SETTING_REGISTERATION(float, IKTermWeight, 1.0f)
SETTING_REGISTERATION(float, MarkovTermWeight, 1.0f)
SETTING_REGISTERATION(float, StyleLikelihoodTermWeight, 1.0f)
SETTING_REGISTERATION(float, IKLimitWeight, 1.0)

SETTING_REGISTERATION(float, DebugArmatureThinkness, 0.005f)

SETTING_REGISTERATION(float, RevampLikilyhoodThreshold, 0.05f)

SETTING_REGISTERATION(float, RevampLikilyhoodTimeThreshold, 1.0f)

SETTING_REGISTERATION(bool, DebugView, false)

SETTING_REGISTERATION(bool, ShowCharacterMesh, true)

SETTING_REGISTERATION(bool, MirrowInputX, false)

SETTING_REGISTERATION(bool, UseJointLengthWeight, false)

SETTING_REGISTERATION(bool, UseStylizedIK, true)
SETTING_REGISTERATION(bool, UseVelocity, false)

SETTING_REGISTERATION(float, DefaultTrackerCovierence, 1000000000)
