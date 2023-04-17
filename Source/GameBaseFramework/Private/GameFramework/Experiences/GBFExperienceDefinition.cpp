#include "GameFramework/Experiences/GBFExperienceDefinition.h"

#include "DVEDataValidator.h"
#include "GameFramework/Experiences/GBFExperienceActionSet.h"

#include <GameFeatureAction.h>

#define LOCTEXT_NAMESPACE "GameBaseFrameworkSystem"

#if WITH_EDITOR
EDataValidationResult FGBFExperienceDefinitionActions::IsDataValid( TArray< FText > & validation_errors ) const
{
    return FDVEDataValidator( validation_errors )
        .NoEmptyItem( VALIDATOR_GET_PROPERTY( GameFeaturesToEnable ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( Actions ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( ActionSets ) )
        .Result();
}
#endif

FPrimaryAssetId UGBFExperienceDefinition::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( GetPrimaryAssetType(), GetPackage()->GetFName() );
}

FPrimaryAssetType UGBFExperienceDefinition::GetPrimaryAssetType()
{
    static const FPrimaryAssetType PrimaryAssetType( TEXT( "ExperienceDefinition" ) );
    return PrimaryAssetType;
}

const UGBFExperienceDefinition * UGBFExperienceDefinition::Resolve( UWorld * world ) const
{
    if ( ConditionalActions.IsEmpty() )
    {
        return this;
    }

    auto * new_experience = NewObject< UGBFExperienceDefinition >();
    new_experience->DefaultActions = DefaultActions;

    for ( const auto & conditional_action : ConditionalActions )
    {
        if ( !ensureAlways( conditional_action.Condition != nullptr ) )
        {
            continue;
        }

        if ( conditional_action.Condition->CanApplyActions( world ) )
        {
            new_experience->ActionSets.Append( conditional_action.Actions.ActionSets );
            new_experience->Actions.Append( conditional_action.Actions.Actions );
            new_experience->GameFeaturesToEnable.Append( conditional_action.Actions.GameFeaturesToEnable );
        }
    }

    return new_experience;
}

void UGBFExperienceDefinition::Serialize( FArchive & ar )
{
    if ( ar.IsSaving() )
    {
        const auto add_if_not_empty = [ & ]( auto & array, const auto & other_array ) {
            if ( array.IsEmpty() )
            {
                array.Append( other_array );
            }
        };

        add_if_not_empty( DefaultActions.ActionSets, ActionSets );
        add_if_not_empty( DefaultActions.Actions, Actions );
        add_if_not_empty( DefaultActions.GameFeaturesToEnable, GameFeaturesToEnable );
    }

    Super::Serialize( ar );
}

#if WITH_EDITOR
EDataValidationResult UGBFExperienceDefinition::IsDataValid( TArray< FText > & validation_errors )
{
    auto result = CombineDataValidationResults( Super::IsDataValid( validation_errors ), DefaultActions.IsDataValid( validation_errors ) );

    for ( const auto & conditional_action : ConditionalActions )
    {
        if ( conditional_action.Condition == nullptr )
        {
            validation_errors.Add( FText::FromString( TEXT( "You can't have a null conditional action." ) ) );

            result = EDataValidationResult::Invalid;
        }
        result = CombineDataValidationResults( result, conditional_action.Actions.IsDataValid( validation_errors ) );
    }

    // Make sure users didn't subclass from a BP of this (it's fine and expected to subclass once in BP, just not twice)
    if ( !GetClass()->IsNative() )
    {
        const auto * parent_class = GetClass()->GetSuperClass();

        // Find the native parent
        const auto * first_native_parent = parent_class;
        while ( ( first_native_parent != nullptr ) && !first_native_parent->IsNative() )
        {
            first_native_parent = first_native_parent->GetSuperClass();
        }

        if ( first_native_parent != parent_class )
        {
            validation_errors.Add( FText::Format( LOCTEXT( "ExperienceInheritenceIsUnsupported", "Blueprint subclasses of Blueprint experiences is not currently supported (use composition via ActionSets instead). Parent class was {0} but should be {1}." ),
                FText::AsCultureInvariant( GetPathNameSafe( parent_class ) ),
                FText::AsCultureInvariant( GetPathNameSafe( first_native_parent ) ) ) );
            result = EDataValidationResult::Invalid;
        }
    }

    return result;
}
#endif

#if WITH_EDITORONLY_DATA
void UGBFExperienceDefinition::UpdateAssetBundleData()
{
    Super::UpdateAssetBundleData();

    const auto update_action_bundle_data = [ & ]( const TArray< UGameFeatureAction * > & actions ) {
        for ( auto * action : actions )
        {
            if ( action != nullptr )
            {
                action->AddAdditionalAssetBundleData( AssetBundleData );
            }
        }
    };

    update_action_bundle_data( DefaultActions.Actions );

    for ( const auto & conditional_action : ConditionalActions )
    {
        update_action_bundle_data( conditional_action.Actions.Actions );
    }
}
#endif