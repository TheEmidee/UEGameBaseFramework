#include "GBFTags.h"

#include "Engine/EngineTypes.h"

UE_DEFINE_GAMEPLAY_TAG( GBFTag_WorldSettings_NoSaveGame, TEXT( "WorldSettings.NoSaveGame" ) );

UE_DEFINE_GAMEPLAY_TAG( GBFTag_Gameplay_Movement_Mode_Walking, TEXT( "Gameplay.Movement.Mode.Walking" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_Gameplay_Movement_Mode_Flying, TEXT( "Gameplay.Movement.Mode.Flying" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_Gameplay_Movement_Mode_Falling, TEXT( "Gameplay.Movement.Mode.Falling" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_Gameplay_Movement_Mode_Swimming, TEXT( "Gameplay.Movement.Mode.Swimming" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_Gameplay_Movement_Mode_NavWalking, TEXT( "Gameplay.Movement.Mode.NavWalking" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_Gameplay_Movement_Mode_Custom, TEXT( "Gameplay.Movement.Mode.Custom" ) );

const TMap< uint8, FGameplayTag > MovementModeTagMap = {
    { MOVE_Walking, GBFTag_Gameplay_Movement_Mode_Walking },
    { MOVE_NavWalking, GBFTag_Gameplay_Movement_Mode_NavWalking },
    { MOVE_Falling, GBFTag_Gameplay_Movement_Mode_Falling },
    { MOVE_Swimming, GBFTag_Gameplay_Movement_Mode_Swimming },
    { MOVE_Flying, GBFTag_Gameplay_Movement_Mode_Flying },
    { MOVE_Custom, GBFTag_Gameplay_Movement_Mode_Custom }
};