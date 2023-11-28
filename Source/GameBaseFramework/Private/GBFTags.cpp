#include "GBFTags.h"

UE_DEFINE_GAMEPLAY_TAG( GBFTag_InitState_Spawned, TEXT( "InitState.Spawned" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_InitState_DataAvailable, TEXT( "InitState.DataAvailable" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_InitState_DataInitialized, TEXT( "InitState.DataInitialized" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_InitState_GameplayReady, TEXT( "InitState.GameplayReady" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_UI_Layer_Game, TEXT( "UI.Layer.Game" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_UI_Layer_GameMenu, TEXT( "UI.Layer.GameMenu" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_UI_Layer_Menu, TEXT( "UI.Layer.Menu" ) );
UE_DEFINE_GAMEPLAY_TAG( GBFTag_UI_Layer_Modal, TEXT( "UI.Layer.Modal" ) );

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