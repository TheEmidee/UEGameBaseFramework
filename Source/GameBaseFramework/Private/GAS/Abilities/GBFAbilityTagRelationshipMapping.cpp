#include "GAS/Abilities/GBFAbilityTagRelationshipMapping.h"

void UGBFAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel( FGameplayTagContainer & tags_to_block, FGameplayTagContainer & tags_to_cancel, const FGameplayTagContainer & ability_tags ) const
{
    for ( const auto & tags : AbilityTagRelationships )
    {
        if ( !ability_tags.HasTag( tags.AbilityTag ) )
        {
            continue;
        }

        tags_to_block.AppendTags( tags.AbilityTagsToBlock );
        tags_to_cancel.AppendTags( tags.AbilityTagsToCancel );
    }
}

void UGBFAbilityTagRelationshipMapping::GetRequiredAndBlockedActivationTags( FGameplayTagContainer & activation_required_tags, FGameplayTagContainer & activation_blocked_tags, const FGameplayTagContainer & ability_tags ) const
{
    for ( const auto & tags : AbilityTagRelationships )
    {
        if ( !ability_tags.HasTag( tags.AbilityTag ) )
        {
            continue;
        }

        activation_required_tags.AppendTags( tags.ActivationRequiredTags );
        activation_blocked_tags.AppendTags( tags.ActivationBlockedTags );
    }
}

bool UGBFAbilityTagRelationshipMapping::IsAbilityCancelledByTag( const FGameplayTagContainer & ability_tags, const FGameplayTag & action_tag ) const
{
    for ( const auto & tags : AbilityTagRelationships )
    {
        if ( tags.AbilityTag == action_tag && tags.AbilityTagsToCancel.HasAny( ability_tags ) )
        {
            return true;
        }
    }

    return false;
}
