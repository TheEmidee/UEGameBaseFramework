#include "Abilities/GBFTaggedActor.h"

void AGBFTaggedActor::GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const
{
    tag_container.AppendTags( StaticGameplayTags );
}

#if WITH_EDITOR
bool AGBFTaggedActor::CanEditChange( const FProperty * in_property ) const
{
    if ( in_property->GetFName() == GET_MEMBER_NAME_CHECKED( AActor, Tags ) )
    {
        return false;
    }

    return Super::CanEditChange( in_property );
}
#endif