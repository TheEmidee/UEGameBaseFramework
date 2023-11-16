#pragma once

#include <CoreMinimal.h>
#include <GameplayCueManager.h>

#include "GASExtGameplayCueManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams( FGASExtOnGameplayCueRoutedDelegate, AActor *, target_actor, FGameplayTag, gameplay_tag, EGameplayCueEvent::Type, event_type, const FGameplayCueParameters &, params );

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtGameplayCueManager : public UGameplayCueManager
{
    GENERATED_BODY()

public:
    FGASExtOnGameplayCueRoutedDelegate & OnGameplayCueRouted();

    void OnCreated() override;
    bool ShouldAsyncLoadRuntimeObjectLibraries() const override;
    bool ShouldSyncLoadMissingGameplayCues() const override;
    bool ShouldAsyncLoadMissingGameplayCues() const override;
    void RouteGameplayCue( AActor * target_actor, FGameplayTag gameplay_cue_tag, EGameplayCueEvent::Type event_type, const FGameplayCueParameters & parameters, EGameplayCueExecutionOptions options ) override;

    void LoadAlwaysLoadedCues();
    void RefreshGameplayCuePrimaryAsset();

    static void DumpGameplayCues( const TArray< FString > & args );
    static UGASExtGameplayCueManager * Get();

private:
    struct FLoadedGameplayTagToProcessData
    {
        FLoadedGameplayTagToProcessData()
        {}
        FLoadedGameplayTagToProcessData( const FGameplayTag & tag, const TWeakObjectPtr< UObject > & owner ) :
            Tag( tag ),
            WeakOwner( owner )
        {}

        FGameplayTag Tag;
        TWeakObjectPtr< UObject > WeakOwner;
    };

    bool ShouldDelayLoadGameplayCues() const;
    void UpdateDelayLoadDelegateListeners();
    void OnGameplayTagLoaded( const FGameplayTag & tag );
    void HandlePostGarbageCollect();
    void HandlePostLoadMap( UWorld * world );
    void ProcessLoadedTags();
    void ProcessTagToPreload( const FGameplayTag & tag, UObject * owning_object );
    void OnPreloadCueComplete( FSoftObjectPath Path, TWeakObjectPtr< UObject > OwningObject, bool bAlwaysLoadedCue );
    void RegisterPreloadedCue( UClass * LoadedGameplayCueClass, UObject * OwningObject );

    // Cues that were preloaded on the client due to being referenced by content
    UPROPERTY( transient )
    TSet< UClass * > PreloadedCues;
    TMap< FObjectKey, TSet< FObjectKey > > PreloadedCueReferencers;

    // Cues that were preloaded on the client and will always be loaded (code referenced or explicitly always loaded)
    UPROPERTY( transient )
    TSet< UClass * > AlwaysLoadedCues;

    FCriticalSection LoadedGameplayTagsToProcessCS;
    TArray< FLoadedGameplayTagToProcessData > LoadedGameplayTagsToProcess;
    bool bProcessLoadedTagsAfterGC = false;

    FGASExtOnGameplayCueRoutedDelegate OnGameplayCueRoutedDelegate;
};

FORCEINLINE FGASExtOnGameplayCueRoutedDelegate & UGASExtGameplayCueManager::OnGameplayCueRouted()
{
    return OnGameplayCueRoutedDelegate;
}