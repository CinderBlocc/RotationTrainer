#include "RotationTrainer.h"
#include "Checkpoints/BackboardCheckpoint.h"
#include "Checkpoints/BallCheckpoint.h"
#include "Checkpoints/BoostCheckpoints.h"
#include "Checkpoints/PostCheckpoint.h"

void RotationTrainer::CreateAllCheckpoints()
{
    AllCheckpoints.clear();

    //Backboard
    AllCheckpoints.push_back(std::make_shared<BackboardCheckpoint>(Vector{ 955.f, 5120.f, 128.f}, Vector{ 2800.f, 5120.f, 700.f}, EAnimationDirection::ANIM_CW, "BL", ELocation::LOC_BACKBOARD_LEFT,  "Backboard Left"));
    AllCheckpoints.push_back(std::make_shared<BackboardCheckpoint>(Vector{-955.f, 5120.f, 128.f}, Vector{-2800.f, 5120.f, 700.f}, EAnimationDirection::ANIM_CW, "BR", ELocation::LOC_BACKBOARD_RIGHT, "Backboard Right"));

    //Post
    AllCheckpoints.push_back(std::make_shared<PostCheckpoint>(Vector{ 955.f, -5120.f, 10.f}, Vector{ 2500.f, -5120.f, 1450.f}, EAnimationDirection::ANIM_CW,  "PL", ELocation::LOC_POST_LEFT,  "Post Left"));
    AllCheckpoints.push_back(std::make_shared<PostCheckpoint>(Vector{-955.f, -5120.f, 10.f}, Vector{-2500.f, -5120.f, 1450.f}, EAnimationDirection::ANIM_CCW, "PR", ELocation::LOC_POST_RIGHT, "Post Right"));

    //Large Boosts
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{ 3072.f, -4096.f, 73.f}, "BLBL", ELocation::LOC_LBOOST_BACK_LEFT,      "Boost Large Back Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{-3072.f, -4096.f, 73.f}, "BLBR", ELocation::LOC_LBOOST_BACK_RIGHT,     "Boost Large Back Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{ 3584.f,  0.f,    73.f}, "BLML", ELocation::LOC_LBOOST_MIDFIELD_LEFT,  "Boost Large Midfield Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{-3584.f,  0.f,    73.f}, "BLMR", ELocation::LOC_LBOOST_MIDFIELD_RIGHT, "Boost Large Midfield Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{ 3072.f,  4096.f, 73.f}, "BLFL", ELocation::LOC_LBOOST_FRONT_LEFT,     "Boost Large Front Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointLarge>(Vector{-3072.f,  4096.f, 73.f}, "BLFR", ELocation::LOC_LBOOST_FRONT_RIGHT,    "Boost Large Front Right"));

    //Small Boosts
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1792.f, -4184.f, 70.f}, "BSBGLL", ELocation::LOC_SBOOST_BACK_GOALLINE_LEFT,        "Boost Small Back Goal Line Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,    -4240.f, 70.f}, "BSBGLC", ELocation::LOC_SBOOST_BACK_GOALLINE_CENTER,      "Boost Small Back Goal Line Center"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1792.f, -4184.f, 70.f}, "BSBGLR", ELocation::LOC_SBOOST_BACK_GOALLINE_RIGHT,       "Boost Small Back Goal Line Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 940.f,  -3308.f, 70.f}, "BSBGBL", ELocation::LOC_SBOOST_BACK_GOALBOX_LEFT,         "Boost Small Back Goal Box Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,    -2816.f, 70.f}, "BSBGBC", ELocation::LOC_SBOOST_BACK_GOALBOX_CENTER,       "Boost Small Back Goal Box Center"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-940.f,  -3308.f, 70.f}, "BSBGBR", ELocation::LOC_SBOOST_BACK_GOALBOX_RIGHT,        "Boost Small Back Goal Box Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 3584.f, -2484.f, 70.f}, "BSBMFL", ELocation::LOC_SBOOST_BACK_MIDDLE_FAR_LEFT,      "Boost Small Back Middle Far Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1788.f, -2300.f, 70.f}, "BSBMCL", ELocation::LOC_SBOOST_BACK_MIDDLE_CENTER_LEFT,   "Boost Small Back Middle Center Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1788.f, -2300.f, 70.f}, "BSBMCR", ELocation::LOC_SBOOST_BACK_MIDDLE_CENTER_RIGHT,  "Boost Small Back Middle Center Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-3584.f, -2484.f, 70.f}, "BSBMFR", ELocation::LOC_SBOOST_BACK_MIDDLE_FAR_RIGHT,     "Boost Small Back Middle Far Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 2048.f, -1036.f, 70.f}, "BSBML",  ELocation::LOC_SBOOST_BACK_MIDFIELD_LEFT,        "Boost Small Back Midfield Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,    -1024.f, 70.f}, "BSBMC",  ELocation::LOC_SBOOST_BACK_MIDFIELD_CENTER,      "Boost Small Back Midfield Center"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-2048.f, -1036.f, 70.f}, "BSBMR",  ELocation::LOC_SBOOST_BACK_MIDFIELD_RIGHT,       "Boost Small Back Midfield Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1024.f,  0.f,    70.f}, "BSML",   ELocation::LOC_SBOOST_MIDFIELD_LEFT,             "Boost Small Midfield Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1024.f,  0.f,    70.f}, "BSMR",   ELocation::LOC_SBOOST_MIDFIELD_RIGHT,            "Boost Small Midfield Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-2048.f,  1036.f, 70.f}, "BSFMR",  ELocation::LOC_SBOOST_FRONT_MIDFIELD_RIGHT,      "Boost Small Front Midfield Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,     1024.f, 70.f}, "BSFMC",  ELocation::LOC_SBOOST_FRONT_MIDFIELD_CENTER,     "Boost Small Front Midfield Center"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 2048.f,  1036.f, 70.f}, "BSFML",  ELocation::LOC_SBOOST_FRONT_MIDFIELD_LEFT,       "Boost Small Front Midfield Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-3584.f,  2484.f, 70.f}, "BSFMFR", ELocation::LOC_SBOOST_FRONT_MIDDLE_FAR_RIGHT,    "Boost Small Front Middle Far Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1788.f,  2300.f, 70.f}, "BSFMCR", ELocation::LOC_SBOOST_FRONT_MIDDLE_CENTER_RIGHT, "Boost Small Front Middle Center Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1788.f,  2300.f, 70.f}, "BSFMCL", ELocation::LOC_SBOOST_FRONT_MIDDLE_CENTER_LEFT,  "Boost Small Front Middle Center Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 3584.f,  2484.f, 70.f}, "BSFMFL", ELocation::LOC_SBOOST_FRONT_MIDDLE_FAR_LEFT,     "Boost Small Front Middle Far Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-940.f,   3308.f, 70.f}, "BSFGBR", ELocation::LOC_SBOOST_FRONT_GOALBOX_RIGHT,       "Boost Small Front Goal Box Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,     2816.f, 70.f}, "BSFGBC", ELocation::LOC_SBOOST_FRONT_GOALBOX_CENTER,      "Boost Small Front Goal Box Center"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 940.f,   3308.f, 70.f}, "BSFGBL", ELocation::LOC_SBOOST_FRONT_GOALBOX_LEFT,        "Boost Small Front Goal Box Left"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{-1792.f,  4184.f, 70.f}, "BSFGLR", ELocation::LOC_SBOOST_FRONT_GOALLINE_RIGHT,      "Boost Small Front Goal Line Right"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 0.f,     4240.f, 70.f}, "BSFGLC", ELocation::LOC_SBOOST_FRONT_GOALLINE_CENTER,     "Boost Small Front Goal Line Center"));
    AllCheckpoints.push_back(std::make_shared<BoostCheckpointSmall>(Vector{ 1792.f,  4184.f, 70.f}, "BSFGLL", ELocation::LOC_SBOOST_FRONT_GOALLINE_LEFT,       "Boost Small Front Goal Line Left"));

    //Ball
    AllCheckpoints.push_back(std::make_shared<BallCheckpoint>("Ball"));
}
