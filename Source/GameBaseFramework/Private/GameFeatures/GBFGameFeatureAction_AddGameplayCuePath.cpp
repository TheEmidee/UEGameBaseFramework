#include "GameFeatures/GBFGameFeatureAction_AddGameplayCuePath.h"

#include "GAS/GameplayCues/GBFGameplayCueManager.h"

#include <AbilitySystemGlobals.h>
#include <GameFeatureData.h>
#include <GameFeaturesSubsystem.h>
#include <GameplayCueSet.h>

#if WITH_EDITOR
#include <Misc/DataValidation.h>
#endif

#define LOCTEXT_NAMESPACE "GameFeatures"

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_AddGameplayCuePath::IsDataValid( FDataValidationContext & context ) const
{
    auto result = Super::IsDataValid( context );

    const auto error_reason = FText::GetEmpty();
    for ( const auto & directory : DirectoryPathsToAdd )
    {
        if ( directory.Path.IsEmpty() )
        {
            const auto invalid_cue_path_error = FText::Format( LOCTEXT( "invalid_cue_path_error", "'{0}' is not a valid path!" ), FText::FromString( directory.Path ) );
            context.AddError( invalid_cue_path_error );
            context.AddError( error_reason );
            result = CombineDataValidationResults( result, EDataValidationResult::Invalid );
        }
    }

    return CombineDataValidationResults( result, EDataValidationResult::Valid );
}
#endif

void UGBFGameFeatureObserver_AddGameplayCuePath::OnGameFeatureRegistering( const UGameFeatureData * game_feature_data, const FString & plugin_name, const FString & /*plugin_url*/ )
{
    TRACE_CPUPROFILER_EVENT_SCOPE( ULyraGameFeature_AddGameplayCuePaths::OnGameFeatureRegistering );

    const auto plugin_root_path = TEXT( "/" ) + plugin_name;
    for ( const auto * action : game_feature_data->GetActions() )
    {
        if ( const auto * game_feature_action_add_gameplay_cue_path = Cast< UGBFGameFeatureAction_AddGameplayCuePath >( action ) )
        {
            const auto & dirs_to_add = game_feature_action_add_gameplay_cue_path->GetDirectoryPathsToAdd();

            if ( auto * gameplay_cue_manager = UGBFGameplayCueManager::Get() )
            {
                const auto * runtime_gameplay_cue_set = gameplay_cue_manager->GetRuntimeCueSet();
                const auto pre_initialize_num_cues = runtime_gameplay_cue_set ? runtime_gameplay_cue_set->GameplayCueData.Num() : 0;

                for ( const auto & [ path ] : dirs_to_add )
                {
                    auto mutable_path = path;
                    UGameFeaturesSubsystem::FixPluginPackagePath( mutable_path, plugin_root_path, false );
                    gameplay_cue_manager->AddGameplayCueNotifyPath( mutable_path, /** bShouldRescanCueAssets = */ false );
                }

                // Rebuild the runtime library with these new paths
                if ( !dirs_to_add.IsEmpty() )
                {
                    gameplay_cue_manager->InitializeRuntimeObjectLibrary();
                }

                if ( const auto post_initialize_num_cues = runtime_gameplay_cue_set
                                                               ? runtime_gameplay_cue_set->GameplayCueData.Num()
                                                               : 0;
                     pre_initialize_num_cues != post_initialize_num_cues )
                {
                    gameplay_cue_manager->RefreshGameplayCuePrimaryAsset();
                }
            }
        }
    }
}

void UGBFGameFeatureObserver_AddGameplayCuePath::OnGameFeatureUnregistering( const UGameFeatureData * game_feature_data, const FString & plugin_name, const FString & /*plugin_url*/ )
{
    const auto plugin_root_path = TEXT( "/" ) + plugin_name;
    for ( const UGameFeatureAction * action : game_feature_data->GetActions() )
    {
        if ( const auto * game_feature_action_add_gameplay_cue_path = Cast< UGBFGameFeatureAction_AddGameplayCuePath >( action ) )
        {
            const auto & dirs_to_add = game_feature_action_add_gameplay_cue_path->GetDirectoryPathsToAdd();

            if ( auto * gcm = UAbilitySystemGlobals::Get().GetGameplayCueManager() )
            {
                int32 num_removed = 0;
                for ( const auto & [ path ] : dirs_to_add )
                {
                    auto mutable_path = path;
                    UGameFeaturesSubsystem::FixPluginPackagePath( mutable_path, plugin_root_path, false );
                    num_removed += gcm->RemoveGameplayCueNotifyPath( mutable_path, /** bShouldRescanCueAssets = */ false );
                }

                ensure( num_removed == dirs_to_add.Num() );

                // Rebuild the runtime library only if there is a need to
                if ( num_removed > 0 )
                {
                    gcm->InitializeRuntimeObjectLibrary();
                }
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE