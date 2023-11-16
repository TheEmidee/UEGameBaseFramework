#include "Engine/GBFAssetManager.h"

#include "Characters/GBFPawnData.h"
#include "GAS/GameplayCues/GASExtGameplayCueManager.h"
#include "GBFLog.h"

#include <AbilitySystemGlobals.h>
#include <Engine/Engine.h>
#include <Misc/App.h>
#include <Misc/ScopedSlowTask.h>
#include <Stats/StatsMisc.h>

static FAutoConsoleCommand CVarDumpLoadedAssets(
    TEXT( "GBF.DumpLoadedAssets" ),
    TEXT( "Shows all assets that were loaded via the asset manager and are currently in memory." ),
    FConsoleCommandDelegate::CreateStatic( UGBFAssetManager::DumpLoadedAssets ) );

const UGBFPawnData * UGBFAssetManager::GetDefaultPawnData() const
{
    return GetAsset( DefaultPawnDataPath );
}

void UGBFAssetManager::DumpLoadedAssets()
{
    UE_LOG( LogGBF, Log, TEXT( "========== Start Dumping Loaded Assets ==========" ) );

    for ( const auto * loaded_asset : Get().LoadedAssets )
    {
        UE_LOG( LogGBF, Log, TEXT( "  %s" ), *GetNameSafe( loaded_asset ) );
    }

    UE_LOG( LogGBF, Log, TEXT( "... %d assets in loaded pool" ), Get().LoadedAssets.Num() );
    UE_LOG( LogGBF, Log, TEXT( "========== Finish Dumping Loaded Assets ==========" ) );
}

UGBFAssetManager & UGBFAssetManager::Get()
{
    check( GEngine );

    if ( auto * singleton = Cast< UGBFAssetManager >( GEngine->AssetManager ) )
    {
        return *singleton;
    }

    UE_LOG( LogGBF, Fatal, TEXT( "Invalid AssetManagerClassName in DefaultEngine.ini.  It must be set to UGBFAssetManager!" ) );

    // Fatal error above prevents this from being called.
    return *NewObject< UGBFAssetManager >();
}

void UGBFAssetManager::AddStartupJob( const FString & job_name, const TFunction< void() > & function, float weight )
{
    StartupJobs.Add( FGBFAssetManagerStartupJob(
        job_name,
        [ this, function ]( const FGBFAssetManagerStartupJob & startup_job, TSharedPtr< FStreamableHandle > & load_handle ) {
            function();
        },
        weight ) );
}

void UGBFAssetManager::LoadGameData()
{
}

void UGBFAssetManager::StartInitialLoading()
{
    SCOPED_BOOT_TIMING( "UGBFAssetManager::StartInitialLoading" );

    // This does all of the scanning, need to do this now even if loads are deferred
    Super::StartInitialLoading();

    AddStartupJob(
        "InitializeGameplayCueManager",
        [ this ]() {
            InitializeGameplayCueManager();
        },
        1.0f );

    // Run all the queued up startup jobs
    DoAllStartupJobs();
}

void UGBFAssetManager::InitializeGameplayCueManager()
{
    SCOPED_BOOT_TIMING( "UGBFAssetManager::InitializeGameplayCueManager" );

    auto * gameplay_cue_manager = UGASExtGameplayCueManager::Get();
    check( gameplay_cue_manager );

    gameplay_cue_manager->LoadAlwaysLoadedCues();
}

void UGBFAssetManager::DoAllStartupJobs()
{
    SCOPED_BOOT_TIMING( "UGBFAssetManager::DoAllStartupJobs" );
    const auto all_startup_jobs_start_time = FPlatformTime::Seconds();

    if ( IsRunningDedicatedServer() )
    {
        // No need for periodic progress updates, just run the jobs
        for ( const auto & startup_job : StartupJobs )
        {
            startup_job.DoJob();
        }
    }
    else
    {
        if ( StartupJobs.Num() > 0 )
        {
            float total_job_value = 0.0f;
            for ( const auto & startup_job : StartupJobs )
            {
                total_job_value += startup_job.JobWeight;
            }

            float accumulated_job_value = 0.0f;

            for ( auto & startup_job : StartupJobs )
            {
                const float job_value = startup_job.JobWeight;
                startup_job.SubstepProgressDelegate.BindLambda( [ This = this, accumulated_job_value, job_value, total_job_value ]( const float new_progress ) {
                    const float substep_adjustment = FMath::Clamp( new_progress, 0.0f, 1.0f ) * job_value;
                    const float overall_percent_with_substep = ( accumulated_job_value + substep_adjustment ) / total_job_value;

                    This->UpdateInitialGameContentLoadPercent( overall_percent_with_substep );
                } );

                startup_job.DoJob();

                startup_job.SubstepProgressDelegate.Unbind();

                accumulated_job_value += job_value;

                UpdateInitialGameContentLoadPercent( accumulated_job_value / total_job_value );
            }
        }
        else
        {
            UpdateInitialGameContentLoadPercent( 1.0f );
        }
    }

    StartupJobs.Empty();

    UE_LOG( LogGBF, Display, TEXT( "All startup jobs took %.2f seconds to complete" ), FPlatformTime::Seconds() - all_startup_jobs_start_time );
}

void UGBFAssetManager::UpdateInitialGameContentLoadPercent( float /*game_content_percent*/ )
{
    // Could route this to the early startup loading screen
}

UPrimaryDataAsset * UGBFAssetManager::LoadGameDataOfClass( TSubclassOf< UPrimaryDataAsset > data_class, const TSoftObjectPtr< UPrimaryDataAsset > & data_class_path, FPrimaryAssetType primary_asset_type )
{
    UPrimaryDataAsset * Asset = nullptr;

    DECLARE_SCOPE_CYCLE_COUNTER( TEXT( "Loading GameData Object" ), STAT_GameData, STATGROUP_LoadTime );
    if ( !data_class_path.IsNull() )
    {
#if WITH_EDITOR
        FScopedSlowTask SlowTask( 0, FText::Format( NSLOCTEXT( "SwarmsEditor", "BeginLoadingGameDataTask", "Loading GameData {0}" ), FText::FromName( data_class->GetFName() ) ) );
        constexpr bool show_cancel_button = false;
        constexpr bool allow_in_pie = true;
        SlowTask.MakeDialog( show_cancel_button, allow_in_pie );

#endif
        UE_LOG( LogGBF, Log, TEXT( "Loading GameData: %s ..." ), *data_class_path.ToString() );
        SCOPE_LOG_TIME_IN_SECONDS( TEXT( "    ... GameData loaded!" ), nullptr );

        // This can be called recursively in the editor because it is called on demand from PostLoad so force a sync load for primary asset and async load the rest in that case
        if ( GIsEditor )
        {
            Asset = data_class_path.LoadSynchronous();
            LoadPrimaryAssetsWithType( primary_asset_type );
        }
        else
        {
            const auto handle = LoadPrimaryAssetsWithType( primary_asset_type );
            if ( handle.IsValid() )
            {
                handle->WaitUntilComplete( 0.0f, false );

                // This should always work
                Asset = Cast< UPrimaryDataAsset >( handle->GetLoadedAsset() );
            }
        }
    }

    if ( Asset != nullptr )
    {
        GameDataMap.Add( data_class, Asset );
    }
    else
    {
        // It is not acceptable to fail to load any GameData asset. It will result in soft failures that are hard to diagnose.
        UE_LOG( LogGBF, Fatal, TEXT( "Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s." ), *data_class_path.ToString(), *primary_asset_type.ToString(), FApp::GetProjectName() );
    }

    return Asset;
}

#if WITH_EDITOR
void UGBFAssetManager::PreBeginPIE( bool start_simulate )
{
    Super::PreBeginPIE( start_simulate );

    {
        FScopedSlowTask scoped_slow_task( 0, NSLOCTEXT( "SwarmsEditor", "BeginLoadingPIEData", "Loading PIE Data" ) );
        const bool show_cancel_button = false;
        const bool allow_in_pie = true;
        scoped_slow_task.MakeDialog( show_cancel_button, allow_in_pie );

        LoadGameData();

        // Intentionally after GetGameData to avoid counting GameData time in this timer
        SCOPE_LOG_TIME_IN_SECONDS( TEXT( "PreBeginPIE asset preloading complete" ), nullptr );

        // You could add preloading of anything else needed for the experience we'll be using here
        // (e.g., by grabbing the default experience from the world settings + the experience override in developer settings)
    }
}
#endif

void UGBFAssetManager::AddLoadedAsset( const UObject * asset )
{
    if ( ensureAlways( asset ) )
    {
        FScopeLock lock( &LoadedAssetsCritical );
        LoadedAssets.Add( asset );
    }
}

UObject * UGBFAssetManager::SynchronousLoadAsset( const FSoftObjectPath & asset_path )
{
    if ( asset_path.IsValid() )
    {
        // ReSharper disable once CppTooWideScope
        // ReSharper disable once CppJoinDeclarationAndAssignment
        TUniquePtr< FScopeLogTime > log_time_ptr;

        if ( ShouldLogAssetLoads() )
        {
            log_time_ptr = MakeUnique< FScopeLogTime >( *FString::Printf( TEXT( "Synchronously loaded asset [%s]" ), *asset_path.ToString() ), nullptr, FScopeLogTime::ScopeLog_Seconds );
        }

        if ( UAssetManager::IsInitialized() )
        {
            return UAssetManager::GetStreamableManager().LoadSynchronous( asset_path, false );
        }

        // Use LoadObject if asset manager isn't ready yet.
        return asset_path.TryLoad();
    }

    return nullptr;
}

bool UGBFAssetManager::ShouldLogAssetLoads()
{
    static auto log_asset_loads = FParse::Param( FCommandLine::Get(), TEXT( "LogAssetLoads" ) );
    return log_asset_loads;
}