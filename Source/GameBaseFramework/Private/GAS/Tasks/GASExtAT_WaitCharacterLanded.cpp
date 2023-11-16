#include "GAS/Tasks/GASExtAT_WaitCharacterLanded.h"

UGASExtAT_WaitCharacterLanded * UGASExtAT_WaitCharacterLanded::WaitCharacterLanded( UGameplayAbility * owning_ability, bool end_on_landed /*= true*/ )
{
    auto * my_obj = NewAbilityTask< UGASExtAT_WaitCharacterLanded >( owning_ability );
    my_obj->bEndOnLanded = end_on_landed;
    return my_obj;
}

void UGASExtAT_WaitCharacterLanded::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( auto * character = Cast< ACharacter >( GetAvatarActor() ) )
    {
        character->LandedDelegate.AddDynamic( this, &UGASExtAT_WaitCharacterLanded::OnCharacterLanded );
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGASExtAT_WaitCharacterLanded::OnCharacterLanded( const FHitResult & hit_result )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnCharacterLandedDelegate.Broadcast( hit_result );
    }

    if ( bEndOnLanded )
    {
        EndTask();
    }
}

void UGASExtAT_WaitCharacterLanded::OnDestroy( const bool ability_ended )
{
    if ( auto * character = Cast< ACharacter >( GetAvatarActor() ) )
    {
        character->LandedDelegate.RemoveDynamic( this, &UGASExtAT_WaitCharacterLanded::OnCharacterLanded );
    }

    Super::OnDestroy( ability_ended );
}
