#include "GameFramework/Experiences/GBFExperienceDefinition.h"

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

const TArray< FString > & UGBFExperienceDefinition::GetAllGameFeatures( const UWorld * world ) const
{
    if ( OptionToAdditionalFeaturesAndActionsMap.IsEmpty() || !IsValid( world ) )
    {
        return GameFeaturesToEnable;
    }

    auto result = GameFeaturesToEnable;
    const auto url = world->GetLocalURL();

    for ( auto option_pair : OptionToAdditionalFeaturesAndActionsMap )
    {
        if ( url.Contains( option_pair.Key ) )
        {
            result.Append( option_pair.Value.GameFeatures );
        }
    }

    return result;
}

const TArray< UGameFeatureAction * > & UGBFExperienceDefinition::GetAllActions( const UWorld * world ) const
{
    if ( OptionToAdditionalFeaturesAndActionsMap.IsEmpty() || !IsValid( world ) )
    {
        return Actions;
    }

    auto result = Actions;
    const auto url = world->GetLocalURL();

    for ( auto option_pair : OptionToAdditionalFeaturesAndActionsMap )
    {
        if ( url.Contains( option_pair.Key ) )
        {
            result.Append( option_pair.Value.Actions );
        }
    }

    return result;
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