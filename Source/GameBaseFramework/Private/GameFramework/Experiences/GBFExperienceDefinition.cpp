#include "GameFramework/Experiences/GBFExperienceDefinition.h"

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

#include "GBFLog.h"
#include "GameFramework/Experiences/GBFExperienceActionSet.h"

#include <Engine/Engine.h>
#include <Engine/World.h>
#include <GameFeatureAction.h>
#include <GameFramework/GameModeBase.h>
#include <Kismet/GameplayStatics.h>
#include <Net/UnrealNetwork.h>
#include <UObject/Package.h>

#define LOCTEXT_NAMESPACE "GameBaseFrameworkSystem"

#if WITH_EDITOR
EDataValidationResult FGBFExperienceDefinitionActions::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .NoEmptyItem( VALIDATOR_GET_PROPERTY( GameFeaturesToEnable ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( Actions ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( ActionSets ) )
        .Result();
}
#endif

bool UGBFExperienceCondition::CanApplyActions_Implementation( UWorld * world ) const
{
    return false;
}

bool UGBFExperienceCondition_HasCommandLineOption::CanApplyActions_Implementation( UWorld * world ) const
{
    return UGameplayStatics::HasOption( world->GetAuthGameMode()->OptionsString, Option );
}

bool UGBFExperienceCondition_DoesNotHaveCommandLineOption::CanApplyActions_Implementation( UWorld * world ) const
{
    return !Super::CanApplyActions_Implementation( world );
}

FGBFExperienceConditionalActions::FGBFExperienceConditionalActions() :
    Type( EGBFExperienceConditionalActionType::Append )
{
}

void UGBFExperienceImplementation::DumpToLog() const
{
    UE_LOG( LogGBF_Experience, Log, TEXT( "*** Experience definition details *** " ) );
    UE_LOG( LogGBF_Experience, Log, TEXT( "GameFeaturesToEnable : " ) );
    for ( const auto & game_feature : GameFeaturesToEnable )
    {
        UE_LOG( LogGBF_Experience, Log, TEXT( "* %s" ), *game_feature );
    }

    const auto append_objects = [ & ]( const auto & objects ) {
        for ( const auto * object : objects )
        {
            UE_LOG( LogGBF_Experience, Log, TEXT( "* %s" ), *GetNameSafe( object ) );
        }
    };

    UE_LOG( LogGBF_Experience, Log, TEXT( "ActionSets : " ) );
    append_objects( ActionSets );
    UE_LOG( LogGBF_Experience, Log, TEXT( "Actions: " ) );
    append_objects( Actions );
}

bool UGBFExperienceImplementation::IsSupportedForNetworking() const
{
    return true;
}

void UGBFExperienceImplementation::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, GameFeaturesToEnable );
    DOREPLIFETIME( ThisClass, Actions );
    DOREPLIFETIME( ThisClass, ActionSets );
    DOREPLIFETIME( ThisClass, DefaultPawnData );
}

FPrimaryAssetId UGBFExperienceDefinition::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( GetPrimaryAssetType(), GetPackage()->GetFName() );
}

UGBFExperienceImplementation * UGBFExperienceDefinition::Resolve( UObject * owner ) const
{
    auto * implementation = NewObject< UGBFExperienceImplementation >( owner );

    const auto append_to_array = []( auto & array, const auto & other_array ) {
        array.Append( other_array );
    };

    implementation->OriginalExperienceDefinition = GetClass();
    implementation->DefaultPawnData = DefaultPawnData;
    append_to_array( implementation->Actions, DefaultActions.Actions );
    append_to_array( implementation->ActionSets, DefaultActions.ActionSets );
    append_to_array( implementation->GameFeaturesToEnable, DefaultActions.GameFeaturesToEnable );

    for ( const auto & conditional_action : ConditionalActions )
    {
        if ( !ensureAlways( conditional_action.Condition != nullptr ) )
        {
            continue;
        }

        auto * world = GEngine->GetWorldFromContextObject( owner, EGetWorldErrorMode::LogAndReturnNull );

        if ( conditional_action.Condition->CanApplyActions( world ) )
        {
            switch ( conditional_action.Type )
            {
                case EGBFExperienceConditionalActionType::Append:
                {
                    append_to_array( implementation->ActionSets, conditional_action.Actions.ActionSets );
                    append_to_array( implementation->Actions, conditional_action.Actions.Actions );
                    append_to_array( implementation->GameFeaturesToEnable, conditional_action.Actions.GameFeaturesToEnable );
                }
                break;
                case EGBFExperienceConditionalActionType::Remove:
                {
                    const auto remove_from_array = []( auto & array, const auto & other_array ) {
                        for ( const auto & other_item : other_array )
                        {
                            array.Remove( other_item );
                        }
                    };

                    remove_from_array( implementation->ActionSets, conditional_action.Actions.ActionSets );
                    remove_from_array( implementation->Actions, conditional_action.Actions.Actions );
                    remove_from_array( implementation->GameFeaturesToEnable, conditional_action.Actions.GameFeaturesToEnable );
                }
                break;
                default:
                {
                    checkNoEntry();
                };
            }
        }
    }

    return implementation;
}

#if WITH_EDITOR
EDataValidationResult UGBFExperienceDefinition::IsDataValid( FDataValidationContext & context ) const
{
    auto result = CombineDataValidationResults( Super::IsDataValid( context ), DefaultActions.IsDataValid( context ) );

    for ( const auto & conditional_action : ConditionalActions )
    {
        if ( conditional_action.Condition == nullptr )
        {
            context.AddError( FText::FromString( TEXT( "You can't have a null conditional action." ) ) );

            result = EDataValidationResult::Invalid;
        }
        result = CombineDataValidationResults( result, conditional_action.Actions.IsDataValid( context ) );
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
            context.AddError( FText::Format( LOCTEXT( "ExperienceInheritenceIsUnsupported", "Blueprint subclasses of Blueprint experiences is not currently supported (use composition via ActionSets instead). Parent class was {0} but should be {1}." ),
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

FPrimaryAssetType UGBFExperienceDefinition::GetPrimaryAssetType()
{
    static const FPrimaryAssetType PrimaryAssetType( TEXT( "ExperienceDefinition" ) );
    return PrimaryAssetType;
}

#undef LOCTEXT_NAMESPACE