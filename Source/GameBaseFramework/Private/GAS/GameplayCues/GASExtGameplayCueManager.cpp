#include "GAS/GameplayCues/GASExtGameplayCueManager.h"

#include "GBFLog.h"

#include <AbilitySystemGlobals.h>
#include <Async/Async.h>
#include <Engine/AssetManager.h>
#include <GameplayCueSet.h>
#include <GameplayCue_Types.h>
#include <GameplayTagsManager.h>
#include <UObject/UObjectThreadContext.h>

enum class EGASExtEditorLoadMode
{
    // Loads all cues upfront; longer loading speed in the editor but short PIE times and effects never fail to play
    LoadUpfront,

    // Outside of editor: Async loads as cue tag are registered
    // In editor: Async loads when cues are invoked
    //   Note: This can cause some 'why didn't I see the effect for X' issues in PIE and is good for iteration speed but otherwise bad for designers
    PreloadAsCuesAreReferenced_GameOnly,

    // Async loads as cue tag are registered
    PreloadAsCuesAreReferenced
};

namespace GASExtGameplayCueManagerCvars
{
    static FAutoConsoleCommand CVarDumpGameplayCues(
        TEXT( "GASExt.DumpGameplayCues" ),
        TEXT( "Shows all assets that were loaded via GASExtGameplayCueManager and are currently in memory." ),
        FConsoleCommandWithArgsDelegate::CreateStatic( UGASExtGameplayCueManager::DumpGameplayCues ) );

    static EGASExtEditorLoadMode LoadMode = EGASExtEditorLoadMode::LoadUpfront;

    const FPrimaryAssetType UGASExtAssetManager_GameplayCueRefsType = TEXT( "GameplayCueRefs" );
    const FName UGASExtAssetManager_GameplayCueRefsName = TEXT( "GameplayCueReferences" );
    const FName UGASExtAssetManager_LoadStateClient = FName( TEXT( "Client" ) );
}

const bool bPreloadEvenInEditor = true;

struct FGameplayCueTagThreadSynchronizeGraphTask final : public FAsyncGraphTaskBase
{
    FGameplayCueTagThreadSynchronizeGraphTask( TFunction< void() > && task ) :
        TheTask( MoveTemp( task ) )
    {}

    void DoTask( ENamedThreads::Type CurrentThread, const FGraphEventRef & MyCompletionGraphEvent )
    {
        TheTask();
    }

    ENamedThreads::Type GetDesiredThread()
    {
        return ENamedThreads::GameThread;
    }

    TFunction< void() > TheTask;
};

void UGASExtGameplayCueManager::OnCreated()
{
    Super::OnCreated();

    UpdateDelayLoadDelegateListeners();
}

bool UGASExtGameplayCueManager::ShouldAsyncLoadRuntimeObjectLibraries() const
{
    switch ( GASExtGameplayCueManagerCvars::LoadMode )
    {
        case EGASExtEditorLoadMode::LoadUpfront:
        {
            return true;
        }
        case EGASExtEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
        {
#if WITH_EDITOR
            if ( GIsEditor )
            {
                return false;
            }
#endif
        }

        break;
        case EGASExtEditorLoadMode::PreloadAsCuesAreReferenced:
        {
            break;
        }
        default:
        {
            checkNoEntry();
        }
        break;
    }

    return !ShouldDelayLoadGameplayCues();
}

bool UGASExtGameplayCueManager::ShouldSyncLoadMissingGameplayCues() const
{
    return false;
}

bool UGASExtGameplayCueManager::ShouldAsyncLoadMissingGameplayCues() const
{
    return true;
}

void UGASExtGameplayCueManager::RouteGameplayCue( AActor * target_actor, FGameplayTag gameplay_cue_tag, EGameplayCueEvent::Type event_type, const FGameplayCueParameters & parameters, EGameplayCueExecutionOptions options )
{
    if ( OnGameplayCueRoutedDelegate.IsBound() )
    {
        OnGameplayCueRoutedDelegate.Broadcast( target_actor, gameplay_cue_tag, event_type, parameters );
    }

    Super::RouteGameplayCue( target_actor, gameplay_cue_tag, event_type, parameters, options );
}

void UGASExtGameplayCueManager::LoadAlwaysLoadedCues()
{
    if ( ShouldDelayLoadGameplayCues() )
    {
        const auto & tag_manager = UGameplayTagsManager::Get();

        //@TODO: Try to collect these by filtering GameplayCue. tags out of native gameplay tags?
        TArray< FName > additional_always_loaded_cue_tags;

        for ( const auto & cue_tag_name : additional_always_loaded_cue_tags )
        {
            auto cue_tag = tag_manager.RequestGameplayTag( cue_tag_name, /*ErrorIfNotFound=*/false );

            if ( cue_tag.IsValid() )
            {
                ProcessTagToPreload( cue_tag, nullptr );
            }
            else
            {
                UE_LOG( LogGBF_GAS, Warning, TEXT( "UGASExtGameplayCueManager::AdditionalAlwaysLoadedCueTags contains invalid tag %s" ), *cue_tag_name.ToString() );
            }
        }
    }
}

void UGASExtGameplayCueManager::RefreshGameplayCuePrimaryAsset()
{
    TArray< FSoftObjectPath > cue_paths;
    if ( const auto * runtime_gameplay_cue_set = GetRuntimeCueSet() )
    {
        runtime_gameplay_cue_set->GetSoftObjectPaths( cue_paths );
    }

    FAssetBundleData bundle_data;
    bundle_data.AddBundleAssetsTruncated( GASExtGameplayCueManagerCvars::UGASExtAssetManager_LoadStateClient, cue_paths );
    const FPrimaryAssetId primary_asset_id = FPrimaryAssetId( GASExtGameplayCueManagerCvars::UGASExtAssetManager_GameplayCueRefsType, GASExtGameplayCueManagerCvars::UGASExtAssetManager_GameplayCueRefsName );
    UAssetManager::Get().AddDynamicAsset( primary_asset_id, FSoftObjectPath(), bundle_data );
}

void UGASExtGameplayCueManager::DumpGameplayCues( const TArray< FString > & args )
{
    auto * gameplay_cue_manager = Cast< UGASExtGameplayCueManager >( UAbilitySystemGlobals::Get().GetGameplayCueManager() );
    if ( !gameplay_cue_manager )
    {
        UE_LOG( LogGBF_GAS, Error, TEXT( "DumpGameplayCues failed. No UGASExtGameplayCueManager found." ) );
        return;
    }

    const bool include_refs = args.Contains( TEXT( "Refs" ) );

    UE_LOG( LogGBF_GAS, Log, TEXT( "=========== Dumping Always Loaded Gameplay Cue Notifies ===========" ) );
    for ( const auto * cue_class : gameplay_cue_manager->AlwaysLoadedCues )
    {
        UE_LOG( LogGBF_GAS, Log, TEXT( "  %s" ), *GetPathNameSafe( cue_class ) );
    }

    UE_LOG( LogGBF_GAS, Log, TEXT( "=========== Dumping Preloaded Gameplay Cue Notifies ===========" ) );
    for ( const auto * cue_class : gameplay_cue_manager->PreloadedCues )
    {
        auto * referencer_set = gameplay_cue_manager->PreloadedCueReferencers.Find( cue_class );
        auto num_refs = referencer_set ? referencer_set->Num() : 0;

        UE_LOG( LogGBF_GAS, Log, TEXT( "  %s (%d refs)" ), *GetPathNameSafe( cue_class ), num_refs );

        if ( include_refs && referencer_set )
        {
            for ( const auto & ref : *referencer_set )
            {
                const auto * ref_object = ref.ResolveObjectPtr();
                UE_LOG( LogGBF_GAS, Log, TEXT( "    ^- %s" ), *GetPathNameSafe( ref_object ) );
            }
        }
    }

    UE_LOG( LogGBF_GAS, Log, TEXT( "=========== Dumping Gameplay Cue Notifies loaded on demand ===========" ) );

    auto num_missing_cues_loaded = 0;
    if ( gameplay_cue_manager->RuntimeGameplayCueObjectLibrary.CueSet )
    {
        for ( const auto & cue_data : gameplay_cue_manager->RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData )
        {
            if ( cue_data.LoadedGameplayCueClass && !gameplay_cue_manager->AlwaysLoadedCues.Contains( cue_data.LoadedGameplayCueClass ) && !gameplay_cue_manager->PreloadedCues.Contains( cue_data.LoadedGameplayCueClass ) )
            {
                num_missing_cues_loaded++;
                UE_LOG( LogGBF_GAS, Log, TEXT( "  %s" ), *cue_data.LoadedGameplayCueClass->GetPathName() );
            }
        }
    }

    UE_LOG( LogGBF_GAS, Log, TEXT( "=========== Gameplay Cue Notify summary ===========" ) );
    UE_LOG( LogGBF_GAS, Log, TEXT( "  ... %d cues in always loaded list" ), gameplay_cue_manager->AlwaysLoadedCues.Num() );
    UE_LOG( LogGBF_GAS, Log, TEXT( "  ... %d cues in preloaded list" ), gameplay_cue_manager->PreloadedCues.Num() );
    UE_LOG( LogGBF_GAS, Log, TEXT( "  ... %d cues loaded on demand" ), num_missing_cues_loaded );
    UE_LOG( LogGBF_GAS, Log, TEXT( "  ... %d cues in total" ), gameplay_cue_manager->AlwaysLoadedCues.Num() + gameplay_cue_manager->PreloadedCues.Num() + num_missing_cues_loaded );
}

UGASExtGameplayCueManager * UGASExtGameplayCueManager::Get()
{
    return Cast< UGASExtGameplayCueManager >( UAbilitySystemGlobals::Get().GetGameplayCueManager() );
}

bool UGASExtGameplayCueManager::ShouldDelayLoadGameplayCues() const
{
    constexpr auto client_delay_load_gameplay_cues = true;
    return !IsRunningDedicatedServer() && client_delay_load_gameplay_cues;
}

void UGASExtGameplayCueManager::UpdateDelayLoadDelegateListeners()
{
    UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.RemoveAll( this );
    FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll( this );
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll( this );

    switch ( GASExtGameplayCueManagerCvars::LoadMode )
    {
        case EGASExtEditorLoadMode::LoadUpfront:
        {
            return;
        }
        case EGASExtEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
        {
#if WITH_EDITOR
            if ( GIsEditor )
            {
                return;
            }
#endif
        }
        break;
        case EGASExtEditorLoadMode::PreloadAsCuesAreReferenced:
        {
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }

    UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.AddUObject( this, &ThisClass::OnGameplayTagLoaded );
    FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject( this, &ThisClass::HandlePostGarbageCollect );
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject( this, &ThisClass::HandlePostLoadMap );
}

void UGASExtGameplayCueManager::OnGameplayTagLoaded( const FGameplayTag & tag )
{
    FScopeLock scope_lock( &LoadedGameplayTagsToProcessCS );
    const auto b_start_task = LoadedGameplayTagsToProcess.Num() == 0;

    auto * load_context = FUObjectThreadContext::Get().GetSerializeContext();
    auto * owning_object = load_context ? load_context->SerializedObject : nullptr;

    LoadedGameplayTagsToProcess.Emplace( tag, owning_object );

    if ( b_start_task )
    {
        TGraphTask< FGameplayCueTagThreadSynchronizeGraphTask >::CreateTask().ConstructAndDispatchWhenReady( []() {
            if ( GIsRunning )
            {
                if ( auto * strong_this = Get() )
                {
                    // If we are garbage collecting we cannot call StaticFindObject (or a few other static uobject functions), so we'll just wait until the GC is over and process the tags then
                    if ( IsGarbageCollecting() )
                    {
                        strong_this->bProcessLoadedTagsAfterGC = true;
                    }
                    else
                    {
                        strong_this->ProcessLoadedTags();
                    }
                }
            }
        } );
    }
}

void UGASExtGameplayCueManager::HandlePostGarbageCollect()
{
    if ( bProcessLoadedTagsAfterGC )
    {
        ProcessLoadedTags();
    }

    bProcessLoadedTagsAfterGC = false;
}

void UGASExtGameplayCueManager::HandlePostLoadMap( UWorld * /*world*/ )
{
    if ( RuntimeGameplayCueObjectLibrary.CueSet )
    {
        for ( auto * cue_class : AlwaysLoadedCues )
        {
            RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass( cue_class );
        }

        for ( auto * cue_class : PreloadedCues )
        {
            RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass( cue_class );
        }
    }

    for ( auto cue_iterator = PreloadedCues.CreateIterator(); cue_iterator; ++cue_iterator )
    {
        TSet< FObjectKey > & referencer_set = PreloadedCueReferencers.FindChecked( *cue_iterator );

        for ( auto referencer_iterator = referencer_set.CreateIterator(); referencer_iterator; ++referencer_iterator )
        {
            if ( !referencer_iterator->ResolveObjectPtr() )
            {
                referencer_iterator.RemoveCurrent();
            }
        }
        if ( referencer_set.Num() == 0 )
        {
            PreloadedCueReferencers.Remove( *cue_iterator );
            cue_iterator.RemoveCurrent();
        }
    }
}

void UGASExtGameplayCueManager::ProcessLoadedTags()
{
    TArray< FLoadedGameplayTagToProcessData > TaskLoadedGameplayTagsToProcess;
    {
        // Lock LoadedGameplayTagsToProcess just long enough to make a copy and clear
        FScopeLock scope_lock( &LoadedGameplayTagsToProcessCS );
        TaskLoadedGameplayTagsToProcess = LoadedGameplayTagsToProcess;
        LoadedGameplayTagsToProcess.Empty();
    }

    // This might return during shutdown, and we don't want to proceed if that is the case
    if ( GIsRunning )
    {
        if ( RuntimeGameplayCueObjectLibrary.CueSet )
        {
            for ( const FLoadedGameplayTagToProcessData & LoadedTagData : TaskLoadedGameplayTagsToProcess )
            {
                if ( RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Contains( LoadedTagData.Tag ) )
                {
                    if ( !LoadedTagData.WeakOwner.IsStale() )
                    {
                        ProcessTagToPreload( LoadedTagData.Tag, LoadedTagData.WeakOwner.Get() );
                    }
                }
            }
        }
        else
        {
            UE_LOG( LogGBF_GAS, Warning, TEXT( "UGASExtGameplayCueManager::OnGameplayTagLoaded processed loaded tag(s) but RuntimeGameplayCueObjectLibrary.CueSet was null. Skipping processing." ) );
        }
    }
}

void UGASExtGameplayCueManager::ProcessTagToPreload( const FGameplayTag & tag, UObject * owning_object )
{
    switch ( GASExtGameplayCueManagerCvars::LoadMode )
    {
        case EGASExtEditorLoadMode::LoadUpfront:
        {
            return;
        }
        case EGASExtEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
        {
#if WITH_EDITOR
            if ( GIsEditor )
            {
                return;
            }
#endif
        }
        break;
        case EGASExtEditorLoadMode::PreloadAsCuesAreReferenced:
        {
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }

    check( RuntimeGameplayCueObjectLibrary.CueSet );

    const auto * data_idx = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Find( tag );
    if ( data_idx != nullptr && RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData.IsValidIndex( *data_idx ) )
    {
        const auto & cue_data = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData[ *data_idx ];

        if ( auto * loaded_gameplay_cue_class = FindObject< UClass >( nullptr, *cue_data.GameplayCueNotifyObj.ToString() ) )
        {
            RegisterPreloadedCue( loaded_gameplay_cue_class, owning_object );
        }
        else
        {
            const auto always_loaded_cue = owning_object == nullptr;
            TWeakObjectPtr< UObject > weak_owner = owning_object;
            StreamableManager.RequestAsyncLoad( cue_data.GameplayCueNotifyObj, FStreamableDelegate::CreateUObject( this, &ThisClass::OnPreloadCueComplete, cue_data.GameplayCueNotifyObj, weak_owner, always_loaded_cue ), FStreamableManager::DefaultAsyncLoadPriority, false, false, TEXT( "GameplayCueManager" ) );
        }
    }
}

void UGASExtGameplayCueManager::OnPreloadCueComplete( const FSoftObjectPath path, const TWeakObjectPtr< UObject > owning_object, const bool always_loaded_cue )
{
    if ( always_loaded_cue || owning_object.IsValid() )
    {
        if ( auto * loaded_gameplay_cue_class = Cast< UClass >( path.ResolveObject() ) )
        {
            RegisterPreloadedCue( loaded_gameplay_cue_class, owning_object.Get() );
        }
    }
}

void UGASExtGameplayCueManager::RegisterPreloadedCue( UClass * loaded_gameplay_cue_class, UObject * owning_object )
{
    check( loaded_gameplay_cue_class );

    const auto always_loaded_cue = owning_object == nullptr;

    if ( always_loaded_cue )
    {
        AlwaysLoadedCues.Add( loaded_gameplay_cue_class );
        PreloadedCues.Remove( loaded_gameplay_cue_class );
        PreloadedCueReferencers.Remove( loaded_gameplay_cue_class );
    }
    else if ( ( owning_object != loaded_gameplay_cue_class ) && ( owning_object != loaded_gameplay_cue_class->GetDefaultObject() ) && !AlwaysLoadedCues.Contains( loaded_gameplay_cue_class ) )
    {
        PreloadedCues.Add( loaded_gameplay_cue_class );
        auto & referencer_set = PreloadedCueReferencers.FindOrAdd( loaded_gameplay_cue_class );
        referencer_set.Add( owning_object );
    }
}