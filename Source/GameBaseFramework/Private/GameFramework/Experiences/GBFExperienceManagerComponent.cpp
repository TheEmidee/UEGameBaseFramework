#include "GameFramework/Experiences/GBFExperienceManagerComponent.h"

#include "GBFLog.h"
#include "GameFramework/Experiences/GBFExperienceActionSet.h"
#include "GameFramework/Experiences/GBFExperienceDefinition.h"
#include "GameFramework/Experiences/GBFExperienceSubsystem.h"
#include "GameFramework/Phases/GBFGamePhaseAbility.h"
#include "GameFramework/Phases/GBFGamePhaseSubsystem.h"
#include "Net/UnrealNetwork.h"

#include <Engine/AssetManager.h>
#include <GameFeatureAction.h>
#include <GameFeaturesSubsystem.h>
#include <GameFeaturesSubsystemSettings.h>
#include <TimerManager.h>

namespace GBFConsoleVariables
{
    static float ExperienceLoadRandomDelayMin = 0.0f;
    static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayMin(
        TEXT( "gbf.chaos.ExperienceDelayLoad.MinSecs" ),
        ExperienceLoadRandomDelayMin,
        TEXT( "This value (in seconds) will be added as a delay of load completion of the experience (along with the random value gbf.chaos.ExperienceDelayLoad.RandomSecs)" ),
        ECVF_Default );

    static float ExperienceLoadRandomDelayRange = 0.0f;
    static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayRange(
        TEXT( "gbf.chaos.ExperienceDelayLoad.RandomSecs" ),
        ExperienceLoadRandomDelayRange,
        TEXT( "A random amount of time between 0 and this value (in seconds) will be added as a delay of load completion of the experience (along with the fixed value gbf.chaos.ExperienceDelayLoad.MinSecs)" ),
        ECVF_Default );

    float GetExperienceLoadDelayDuration()
    {
        return FMath::Max( 0.0f, ExperienceLoadRandomDelayMin + FMath::FRand() * ExperienceLoadRandomDelayRange );
    }
}

UGBFExperienceManagerComponent::UGBFExperienceManagerComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    SetIsReplicatedByDefault( true );
}

void UGBFExperienceManagerComponent::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
    Super::EndPlay( EndPlayReason );

    // deactivate any features this experience loaded
    //@TODO: This should be handled FILO as well
    for ( const auto & plugin_url : GameFeaturePluginURLs )
    {
        if ( UGBFExperienceSubsystem::RequestToDeactivatePlugin( plugin_url ) )
        {
            UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin( plugin_url );
        }
    }

    //@TODO: Ensure proper handling of a partially-loaded state too
    if ( LoadState == EGBFExperienceLoadState::Loaded )
    {
        LoadState = EGBFExperienceLoadState::Deactivating;

        // Make sure we won't complete the transition prematurely if someone registers as a pauser but fires immediately
        NumExpectedPausers = INDEX_NONE;
        NumObservedPausers = 0;

        // Deactivate and unload the actions
        FGameFeatureDeactivatingContext context( FSimpleDelegate::CreateUObject( this, &ThisClass::OnActionDeactivationCompleted ) );
        if ( const FWorldContext * existing_world_context = GEngine->GetWorldContextFromWorld( GetWorld() ) )
        {
            context.SetRequiredWorldContextHandle( existing_world_context->ContextHandle );
        }

        auto deactivate_list_of_actions = [ &context ]( const TArray< UGameFeatureAction * > & ActionList ) {
            for ( auto * action : ActionList )
            {
                if ( action != nullptr )
                {
                    action->OnGameFeatureDeactivating( context );
                    action->OnGameFeatureUnregistering();
                }
            }
        };

        TArray< UGameFeatureAction * > actions;
        CurrentExperience->GetAllActions( actions, GEngine->LastURLFromWorld( GetWorld() ).ToString() );

        deactivate_list_of_actions( actions );
        for ( const auto * action_set : CurrentExperience->ActionSets )
        {
            if ( action_set != nullptr )
            {
                deactivate_list_of_actions( action_set->Actions );
            }
        }

        NumExpectedPausers = context.GetNumPausers();

        if ( NumExpectedPausers > 0 )
        {
            UE_LOG( LogGBF_Experience, Error, TEXT( "Actions that have asynchronous deactivation aren't fully supported yet in GBF experiences" ) );
        }

        if ( NumExpectedPausers == NumObservedPausers )
        {
            OnAllActionsDeactivated();
        }
    }
}

#if WITH_SERVER_CODE
void UGBFExperienceManagerComponent::ServerSetCurrentExperience( FPrimaryAssetId ExperienceId )
{
    const UAssetManager & asset_manager = UAssetManager::Get();
    const auto asset_path = asset_manager.GetPrimaryAssetPath( ExperienceId );
    const TSubclassOf< UGBFExperienceDefinition > asset_class = Cast< UClass >( asset_path.TryLoad() );

    check( asset_class );

    const auto * experience = GetDefault< UGBFExperienceDefinition >( asset_class );

    check( experience != nullptr );
    check( CurrentExperience == nullptr );
    CurrentExperience = experience;
    StartExperienceLoad();
}
#endif

void UGBFExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority( FOnGBFExperienceLoaded::FDelegate && delegate )
{
    if ( IsExperienceLoaded() )
    {
        delegate.Execute( CurrentExperience );
    }
    else
    {
        OnExperienceLoaded_HighPriority.Add( MoveTemp( delegate ) );
    }
}

void UGBFExperienceManagerComponent::CallOrRegister_OnExperienceLoaded( FOnGBFExperienceLoaded::FDelegate && delegate )
{
    if ( IsExperienceLoaded() )
    {
        delegate.Execute( CurrentExperience );
    }
    else
    {
        OnExperienceLoaded.Add( MoveTemp( delegate ) );
    }
}

void UGBFExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority( FOnGBFExperienceLoaded::FDelegate && delegate )
{
    if ( IsExperienceLoaded() )
    {
        delegate.Execute( CurrentExperience );
    }
    else
    {
        OnExperienceLoaded_LowPriority.Add( MoveTemp( delegate ) );
    }
}

const UGBFExperienceDefinition * UGBFExperienceManagerComponent::GetCurrentExperienceChecked() const
{
    check( LoadState == EGBFExperienceLoadState::Loaded );
    check( CurrentExperience != nullptr );
    return CurrentExperience;
}

bool UGBFExperienceManagerComponent::IsExperienceLoaded() const
{
    return ( LoadState == EGBFExperienceLoadState::Loaded ) && ( CurrentExperience != nullptr );
}

UGBFExperienceManagerComponent * UGBFExperienceManagerComponent::GetExperienceManagerComponent( const UObject * world_context )
{
    if ( const auto * world = world_context->GetWorld() )
    {
        if ( const auto * gs = world->GetGameState() )
        {
            return gs->FindComponentByClass< UGBFExperienceManagerComponent >();
        }
    }

    return nullptr;
}

void UGBFExperienceManagerComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, CurrentExperience );
}

void UGBFExperienceManagerComponent::OnRep_CurrentExperience()
{
    StartExperienceLoad();
}

void UGBFExperienceManagerComponent::StartExperienceLoad()
{
    check( CurrentExperience != nullptr );
    check( LoadState == EGBFExperienceLoadState::Unloaded );

    UE_LOG( LogGBF_Experience, Log, TEXT( "EXPERIENCE: StartExperienceLoad(CurrentExperience = %s, %s)" ), *CurrentExperience->GetPrimaryAssetId().ToString(), *GetClientServerContextString( this ) );

    LoadState = EGBFExperienceLoadState::Loading;

    auto & asset_manager = UAssetManager::Get();

    TSet< FPrimaryAssetId > bundle_asset_list;
    TSet< FSoftObjectPath > raw_asset_list;

    bundle_asset_list.Add( CurrentExperience->GetPrimaryAssetId() );
    for ( const UGBFExperienceActionSet * action_set : CurrentExperience->ActionSets )
    {
        if ( action_set != nullptr )
        {
            bundle_asset_list.Add( action_set->GetPrimaryAssetId() );
        }
    }

    // Load assets associated with the experience

    TArray< FName > bundles_to_load;

    // :TODO: Keep?
    // bundles_to_load.Add( FLyraBundles::Equipped );

    //@TODO: Centralize this client/server stuff into the LyraAssetManager
    const auto owner_net_mode = GetOwner()->GetNetMode();
    const auto load_client = GIsEditor || ( owner_net_mode != NM_DedicatedServer );
    const auto load_server = GIsEditor || ( owner_net_mode != NM_Client );

    if ( load_client )
    {
        bundles_to_load.Add( UGameFeaturesSubsystemSettings::LoadStateClient );
    }
    if ( load_server )
    {
        bundles_to_load.Add( UGameFeaturesSubsystemSettings::LoadStateServer );
    }

    const auto bundle_load_handle = asset_manager.ChangeBundleStateForPrimaryAssets( bundle_asset_list.Array(), bundles_to_load, {}, false, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority );
    const auto raw_load_handle = asset_manager.LoadAssetList( raw_asset_list.Array(), FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, TEXT( "StartExperienceLoad()" ) );

    // If both async loads are running, combine them
    TSharedPtr< FStreamableHandle > handle = nullptr;
    if ( bundle_load_handle.IsValid() && raw_load_handle.IsValid() )
    {
        handle = asset_manager.GetStreamableManager().CreateCombinedHandle( { bundle_load_handle, raw_load_handle } );
    }
    else
    {
        handle = bundle_load_handle.IsValid() ? bundle_load_handle : raw_load_handle;
    }

    FStreamableDelegate on_assets_loaded_delegate = FStreamableDelegate::CreateUObject( this, &ThisClass::OnExperienceLoadComplete );
    if ( !handle.IsValid() || handle->HasLoadCompleted() )
    {
        // Assets were already loaded, call the delegate now
        FStreamableHandle::ExecuteDelegate( on_assets_loaded_delegate );
    }
    else
    {
        handle->BindCompleteDelegate( on_assets_loaded_delegate );

        handle->BindCancelDelegate( FStreamableDelegate::CreateLambda( [ on_assets_loaded_delegate ]() {
            on_assets_loaded_delegate.ExecuteIfBound();
        } ) );
    }

    // This set of assets gets preloaded, but we don't block the start of the experience based on it
    const TSet< FPrimaryAssetId > preload_asset_list;
    //@TODO: Determine assets to preload (but not blocking-ly)
    if ( preload_asset_list.Num() > 0 )
    {
        asset_manager.ChangeBundleStateForPrimaryAssets( preload_asset_list.Array(), bundles_to_load, {} );
    }
}

void UGBFExperienceManagerComponent::OnExperienceLoadComplete()
{
    check( LoadState == EGBFExperienceLoadState::Loading );
    check( CurrentExperience != nullptr );

    UE_LOG( LogGBF_Experience, Log, TEXT( "EXPERIENCE: OnExperienceLoadComplete(CurrentExperience = %s, %s)" ), *CurrentExperience->GetPrimaryAssetId().ToString(), *GetClientServerContextString( this ) );

    // find the URLs for our GameFeaturePlugins - filtering out dupes and ones that don't have a valid mapping
    GameFeaturePluginURLs.Reset();

    auto collect_game_feature_plugin_urls = [ This = this ]( const UPrimaryDataAsset * context, const TArray< FString > & feature_plugin_list ) {
        for ( const FString & plugin_name : feature_plugin_list )
        {
            FString plugin_url;
            if ( UGameFeaturesSubsystem::Get().GetPluginURLForBuiltInPluginByName( plugin_name, /*out*/ plugin_url ) )
            {
                This->GameFeaturePluginURLs.AddUnique( plugin_url );
            }
            else
            {
                ensureMsgf( false, TEXT( "OnExperienceLoadComplete failed to find plugin URL from PluginName %s for experience %s - fix data, ignoring for this run" ), *plugin_name, *context->GetPrimaryAssetId().ToString() );
            }
        }

        // 		// Add in our extra plugin
        // 		if (!CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent.IsEmpty())
        // 		{
        // 			FString PluginURL;
        // 			if (UGameFeaturesSubsystem::Get().GetPluginURLForBuiltInPluginByName(CurrentPlaylistData->GameFeaturePluginToActivateUntilDownloadedContentIsPresent, PluginURL))
        // 			{
        // 				GameFeaturePluginURLs.AddUnique(PluginURL);
        // 			}
        // 		}
    };

    TArray< FString > game_features;
    CurrentExperience->GetAllGameFeatures( game_features, GetWorld()->GetLocalURL() );

    collect_game_feature_plugin_urls( CurrentExperience, game_features );
    for ( const auto * action_set : CurrentExperience->ActionSets )
    {
        if ( action_set != nullptr )
        {
            collect_game_feature_plugin_urls( action_set, action_set->GameFeaturesToEnable );
        }
    }

    // Load and activate the features
    NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
    if ( NumGameFeaturePluginsLoading > 0 )
    {
        LoadState = EGBFExperienceLoadState::LoadingGameFeatures;
        for ( const auto & plugin_url : GameFeaturePluginURLs )
        {
            UGBFExperienceSubsystem::NotifyOfPluginActivation( plugin_url );
            UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin( plugin_url, FGameFeaturePluginLoadComplete::CreateUObject( this, &ThisClass::OnGameFeaturePluginLoadComplete ) );
        }
    }
    else
    {
        OnExperienceFullLoadCompleted();
    }
}

void UGBFExperienceManagerComponent::OnGameFeaturePluginLoadComplete( const UE::GameFeatures::FResult & /*result*/ )
{
    // decrement the number of plugins that are loading
    NumGameFeaturePluginsLoading--;

    if ( NumGameFeaturePluginsLoading == 0 )
    {
        OnExperienceFullLoadCompleted();
    }
}

void UGBFExperienceManagerComponent::OnExperienceFullLoadCompleted()
{
    check( LoadState != EGBFExperienceLoadState::Loaded );

    // Insert a random delay for testing (if configured)
    if ( LoadState != EGBFExperienceLoadState::LoadingChaosTestingDelay )
    {
        const float DelaySecs = GBFConsoleVariables::GetExperienceLoadDelayDuration();
        if ( DelaySecs > 0.0f )
        {
            FTimerHandle DummyHandle;

            LoadState = EGBFExperienceLoadState::LoadingChaosTestingDelay;
            GetWorld()->GetTimerManager().SetTimer( DummyHandle, this, &ThisClass::OnExperienceFullLoadCompleted, DelaySecs, /*bLooping=*/false );

            return;
        }
    }

    LoadState = EGBFExperienceLoadState::ExecutingActions;

    // Execute the actions
    FGameFeatureActivatingContext context;

    // Only apply to our specific world context if set
    if ( const FWorldContext * existing_world_context = GEngine->GetWorldContextFromWorld( GetWorld() ) )
    {
        context.SetRequiredWorldContextHandle( existing_world_context->ContextHandle );
    }

    auto activate_list_of_actions = [ &context ]( const TArray< UGameFeatureAction * > & action_list ) {
        for ( UGameFeatureAction * action : action_list )
        {
            if ( action != nullptr )
            {
                //@TODO: The fact that these don't take a world are potentially problematic in client-server PIE
                // The current behavior matches systems like gameplay tags where loading and registering apply to the entire process,
                // but actually applying the results to actors is restricted to a specific world
                action->OnGameFeatureRegistering();
                action->OnGameFeatureLoading();
                action->OnGameFeatureActivating( context );
            }
        }
    };

    TArray< UGameFeatureAction * > actions;
    CurrentExperience->GetAllActions( actions, GetWorld()->GetLocalURL() );

    activate_list_of_actions( actions );
    for ( const auto * action_set : CurrentExperience->ActionSets )
    {
        if ( action_set != nullptr )
        {
            activate_list_of_actions( action_set->Actions );
        }
    }

    LoadState = EGBFExperienceLoadState::Loaded;

    OnExperienceLoaded_HighPriority.Broadcast( CurrentExperience );
    OnExperienceLoaded_HighPriority.Clear();

    OnExperienceLoaded.Broadcast( CurrentExperience );
    OnExperienceLoaded.Clear();

    OnExperienceLoaded_LowPriority.Broadcast( CurrentExperience );
    OnExperienceLoaded_LowPriority.Clear();

    // Apply any necessary scalability settings
    // :TODO: UE5 Settings
#if !UE_SERVER
    // ULyraSettingsLocal::Get()->OnExperienceLoaded();
#endif
}

void UGBFExperienceManagerComponent::OnActionDeactivationCompleted()
{
    check( IsInGameThread() );
    ++NumObservedPausers;

    if ( NumObservedPausers == NumExpectedPausers )
    {
        OnAllActionsDeactivated();
    }
}

void UGBFExperienceManagerComponent::OnAllActionsDeactivated()
{
    //@TODO: We actually only deactivated and didn't fully unload...
    LoadState = EGBFExperienceLoadState::Unloaded;
    CurrentExperience = nullptr;
    //@TODO:	GEngine->ForceGarbageCollection(true);
}
