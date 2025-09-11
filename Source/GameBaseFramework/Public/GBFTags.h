#pragma once

#include "NativeGameplayTags.h"

GAMEBASEFRAMEWORK_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( GBFTag_WorldSettings_NoSaveGame );

GAMEBASEFRAMEWORK_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( GBFTag_Gameplay_Movement_Mode_Walking );
GAMEBASEFRAMEWORK_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( GBFTag_Gameplay_Movement_Mode_Flying );
GAMEBASEFRAMEWORK_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( GBFTag_Gameplay_Movement_Mode_Falling );
GAMEBASEFRAMEWORK_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( GBFTag_Gameplay_Movement_Mode_Swimming );
GAMEBASEFRAMEWORK_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( GBFTag_Gameplay_Movement_Mode_NavWalking );
GAMEBASEFRAMEWORK_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( GBFTag_Gameplay_Movement_Mode_Custom );

// These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
GAMEBASEFRAMEWORK_API extern const TMap< uint8, FGameplayTag > MovementModeTagMap;