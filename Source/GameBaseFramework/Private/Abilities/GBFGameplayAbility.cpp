#include "Abilities/GBFGameplayAbility.h"

#include "Camera/GBFCameraMode.h"
#include "Characters/Components/GBFHeroComponent.h"

#include <AbilitySystemLog.h>

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN( FunctionName, ReturnValue )                                                                                \
    {                                                                                                                                                        \
        if ( !ensureAlways( IsInstantiated() ) )                                                                                                             \
        {                                                                                                                                                    \
            ABILITY_LOG( Error, TEXT( "%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy." ), *GetPathName() ); \
            return ReturnValue;                                                                                                                              \
        }                                                                                                                                                    \
    }

UGBFGameplayAbility::UGBFGameplayAbility()
{
    ActiveCameraMode = nullptr;
}

UGBFHeroComponent * UGBFGameplayAbility::GetHeroComponentFromActorInfo() const
{
    return CurrentActorInfo ? UGBFHeroComponent::FindHeroComponent( CurrentActorInfo->AvatarActor.Get() ) : nullptr;
}

void UGBFGameplayAbility::SetCameraMode( TSubclassOf< UGBFCameraMode > camera_mode )
{
    ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN( SetCameraMode, );

    if ( auto * hero_component = GetHeroComponentFromActorInfo() )
    {
        hero_component->SetAbilityCameraMode( camera_mode, CurrentSpecHandle );
        ActiveCameraMode = camera_mode;
    }
}

void UGBFGameplayAbility::ClearCameraMode()
{
    ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN( ClearCameraMode, );

    if ( ActiveCameraMode != nullptr )
    {
        if ( auto * hero_component = GetHeroComponentFromActorInfo() )
        {
            hero_component->ClearAbilityCameraMode( CurrentSpecHandle );
        }

        ActiveCameraMode = nullptr;
    }
}