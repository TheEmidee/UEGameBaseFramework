#include "GAS/Tasks/GBFAT_WaitCharacterLanded.h"

UGBFAT_WaitCharacterLanded * UGBFAT_WaitCharacterLanded::WaitCharacterLanded( UGameplayAbility * owning_ability, bool end_on_landed /*= true*/ )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitCharacterLanded >( owning_ability );
    my_obj->bEndOnLanded = end_on_landed;
    return my_obj;
}

void UGBFAT_WaitCharacterLanded::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( auto * character = Cast< ACharacter >( GetAvatarActor() ) )
    {
        character->LandedDelegate.AddDynamic( this, &UGBFAT_WaitCharacterLanded::OnCharacterLanded );
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFAT_WaitCharacterLanded::OnCharacterLanded( const FHitResult & hit_result )
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

void UGBFAT_WaitCharacterLanded::OnDestroy( const bool ability_ended )
{
    if ( auto * character = Cast< ACharacter >( GetAvatarActor() ) )
    {
        character->LandedDelegate.RemoveDynamic( this, &UGBFAT_WaitCharacterLanded::OnCharacterLanded );
    }

    Super::OnDestroy( ability_ended );
}
