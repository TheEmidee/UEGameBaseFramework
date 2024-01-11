#pragma once

#include <CoreMinimal.h>
#include <GameplayAbilitySpec.h>
#include <GameplayTagContainer.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFGamePhaseSubsystem.generated.h"

class UGBFAbilitySystemComponent;
class UGBFGamePhaseAbility;

DECLARE_LOG_CATEGORY_EXTERN( LogGBFGamePhase, Log, All );

DECLARE_DYNAMIC_DELEGATE_OneParam( FGBFGamePhaseDynamicDelegate, const UGBFGamePhaseAbility *, Phase );
DECLARE_DELEGATE_OneParam( FGBFGamePhaseDelegate, const UGBFGamePhaseAbility * Phase );

DECLARE_DYNAMIC_DELEGATE_OneParam( FGBFGamePhaseTagDynamicDelegate, const FGameplayTag &, PhaseTag );
DECLARE_DELEGATE_OneParam( FGBFGamePhaseTagDelegate, const FGameplayTag & PhaseTag );

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFGamePhaseObserverHandle
{
    GENERATED_USTRUCT_BODY()

    FGBFGamePhaseObserverHandle() :
        Handle( INDEX_NONE )
    {
    }

    explicit FGBFGamePhaseObserverHandle( int32 handle ) :
        Handle( handle )
    {
    }

    bool IsValid() const
    {
        return Handle != INDEX_NONE;
    }

    bool operator==( const FGBFGamePhaseObserverHandle & other ) const
    {
        return Handle == other.Handle;
    }

    bool operator!=( const FGBFGamePhaseObserverHandle & other ) const
    {
        return Handle != other.Handle;
    }

    friend uint32 GetTypeHash( const FGBFGamePhaseObserverHandle & handle )
    {
        return handle.Handle;
    }

    FString ToString() const
    {
        return FString::Printf( TEXT( "%d" ), Handle );
    }

    void Invalidate()
    {
        Handle = INDEX_NONE;
    }

    static FGBFGamePhaseObserverHandle GenerateNewHandle()
    {
        static int GHandle = 0;

        return FGBFGamePhaseObserverHandle( GHandle++ );
    }

private:
    int32 Handle;
};

// Match rule for message receivers
UENUM( BlueprintType )
enum class EPhaseTagMatchType : uint8
{
    // An exact match will only receive messages with exactly the same channel
    // (e.g., registering for "A.B" will match a broadcast of A.B but not A.B.C)
    ExactMatch,

    // A partial match will receive any messages rooted in the same channel
    // (e.g., registering for "A.B" will match a broadcast of A.B as well as A.B.C)
    PartialMatch
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGamePhaseSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    void StartPhase( TSubclassOf< UGBFGamePhaseAbility > phase_ability, FGBFGamePhaseDelegate phase_ended_callback = FGBFGamePhaseDelegate() );

    FGBFGamePhaseObserverHandle WhenPhaseStartsOrIsActive( FGameplayTag phase_tag, EPhaseTagMatchType match_type, const FGBFGamePhaseTagDelegate & when_phase_active, bool trigger_once = false );
    FGBFGamePhaseObserverHandle WhenPhaseEnds( FGameplayTag phase_tag, EPhaseTagMatchType match_type, const FGBFGamePhaseTagDelegate & when_phase_end, bool trigger_once = false );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = false, meta = ( AutoCreateRefTerm = "phase_tag" ) )
    bool IsPhaseActive( const FGameplayTag & phase_tag ) const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase" )
    void UnRegisterObserver( FGBFGamePhaseObserverHandle handle );

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
    void DumpActivePhases( FOutputDevice & output_device );
#endif

protected:
    bool DoesSupportWorldType( const EWorldType::Type world_type ) const override;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = ( DisplayName = "Start Phase", AutoCreateRefTerm = "phase_ended_delegate" ) )
    void K2_StartPhase( TSubclassOf< UGBFGamePhaseAbility > phase_ability, const FGBFGamePhaseDynamicDelegate & phase_ended_delegate );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = ( DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "when_phase_active" ) )
    FGBFGamePhaseObserverHandle K2_WhenPhaseStartsOrIsActive( FGameplayTag phase_tag, EPhaseTagMatchType match_type, FGBFGamePhaseTagDynamicDelegate when_phase_active, bool trigger_once = false );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = ( DisplayName = "When Phase Ends", AutoCreateRefTerm = "when_phase_end" ) )
    FGBFGamePhaseObserverHandle K2_WhenPhaseEnds( FGameplayTag phase_tag, EPhaseTagMatchType match_type, FGBFGamePhaseTagDynamicDelegate when_phase_end, bool trigger_once = false );

    void OnBeginPhase( const UGBFGamePhaseAbility * phase_ability, const FGameplayAbilitySpecHandle phase_ability_handle );
    void OnEndPhase( const UGBFGamePhaseAbility * phase_ability, const FGameplayAbilitySpecHandle phase_ability_handle );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase" )
    void EndAllPhases();

private:
    void GetActivePhases( TArray< FGameplayAbilitySpec * > & active_phases, UGBFAbilitySystemComponent * asc ) const;

    struct FGBFGamePhaseEntry
    {
        FGameplayTag PhaseTag;
        FGBFGamePhaseDelegate PhaseEndedCallback;
    };

    TMap< FGameplayAbilitySpecHandle, FGBFGamePhaseEntry > ActivePhaseMap;

    struct FPhaseObserver
    {
        FPhaseObserver() :
            Handle( FGBFGamePhaseObserverHandle::GenerateNewHandle() ),
            bTriggerOnce( false ) {};

        bool IsMatch( const FGameplayTag & compare_phase_tag ) const;

        FGameplayTag PhaseTag;
        EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
        FGBFGamePhaseTagDelegate PhaseCallback;
        FGBFGamePhaseObserverHandle Handle;
        bool bTriggerOnce;
    };

    TArray< FPhaseObserver > PhaseStartObservers;
    TArray< FPhaseObserver > PhaseEndObservers;

    friend class UGBFGamePhaseAbility;
};
