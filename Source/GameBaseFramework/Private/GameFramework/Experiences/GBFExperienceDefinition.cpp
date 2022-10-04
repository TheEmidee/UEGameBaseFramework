#include "GameFramework/Experiences/GBFExperienceDefinition.h"

#include "GameFramework/Experiences/GBFExperienceActionSet.h"

#include <Engine/World.h>
#include <GameFeatureAction.h>

#define LOCTEXT_NAMESPACE "GameBaseFrameworkSystem"

FPrimaryAssetId UGBFExperienceDefinition::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( GetPrimaryAssetType(), GetPackage()->GetFName() );
}

FPrimaryAssetType UGBFExperienceDefinition::GetPrimaryAssetType()
{
    static const FPrimaryAssetType PrimaryAssetType( TEXT( "ExperienceDefinition" ) );
    return PrimaryAssetType;
}

void UGBFExperienceDefinition::GetAllGameFeatures( TArray< FString > & features, const UWorld * world ) const
{
    features.Reserve( GameFeaturesToEnable.Num() * 2 );
    features.Append( GameFeaturesToEnable );

    for ( const auto * action_set : ActionSets )
    {
        if ( action_set != nullptr )
        {
            features.Append( action_set->GameFeaturesToEnable );
        }
    }

    if ( OptionToAdditionalFeaturesAndActionsMap.IsEmpty() || !IsValid( world ) )
    {
        return;
    }

    const auto url = world->GetLocalURL();

    for ( const auto & [ option, features_and_actions ] : OptionToAdditionalFeaturesAndActionsMap )
    {
        if ( url.Contains( option ) )
        {
            features.Append( features_and_actions.GameFeaturesToEnable );
        }
    }
}

void UGBFExperienceDefinition::GetAllActions( TArray< UGameFeatureAction * > & actions, const UWorld * world ) const
{
    actions.Reserve( Actions.Num() * 2 );
    actions.Append( Actions );

    for ( const auto * action_set : ActionSets )
    {
        if ( action_set != nullptr )
        {
            actions.Append( action_set->Actions );
        }
    }

    if ( OptionToAdditionalFeaturesAndActionsMap.IsEmpty() || !IsValid( world ) )
    {
        return;
    }

    const auto url = world->GetLocalURL();

    for ( const auto & [ option, features_and_actions ] : OptionToAdditionalFeaturesAndActionsMap )
    {
        if ( url.Contains( option ) )
        {
            actions.Append( features_and_actions.Actions );
        }
    }
}

#if WITH_EDITOR
EDataValidationResult UGBFExperienceDefinition::IsDataValid( TArray< FText > & validation_errors )
{
    auto result = CombineDataValidationResults( Super::IsDataValid( validation_errors ), EDataValidationResult::Valid );

    auto entry_index = 0;
    for ( auto * action : Actions )
    {
        if ( action != nullptr )
        {
            const auto child_result = action->IsDataValid( validation_errors );
            result = CombineDataValidationResults( result, child_result );
        }
        else
        {
            result = EDataValidationResult::Invalid;
            validation_errors.Add( FText::Format( LOCTEXT( "ActionEntryIsNull", "Null entry at index {0} in Actions" ), FText::AsNumber( entry_index ) ) );
        }

        ++entry_index;
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

    for ( auto * action : Actions )
    {
        if ( action != nullptr )
        {
            action->AddAdditionalAssetBundleData( AssetBundleData );
        }
    }
}
#endif