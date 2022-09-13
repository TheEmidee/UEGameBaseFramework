#pragma once

#include <CoreMinimal.h>
#include <GameplayAbilitySpec.h>
#include <GameplayTagContainer.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFGamePhaseSubsystem.generated.h"

class UGASExtAbilitySystemComponent;
class UGBFGamePhaseAbility;

DECLARE_LOG_CATEGORY_EXTERN( LogGBFGamePhase, Log, All );

DECLARE_DYNAMIC_DELEGATE_OneParam( FGBFGamePhaseDynamicDelegate, const UGBFGamePhaseAbility *, Phase );
DECLARE_DELEGATE_OneParam( FGBFGamePhaseDelegate, const UGBFGamePhaseAbility * Phase );

DECLARE_DYNAMIC_DELEGATE_OneParam( FGBFGamePhaseTagDynamicDelegate, const FGameplayTag &, PhaseTag );
DECLARE_DELEGATE_OneParam( FGBFGamePhaseTagDelegate, const FGameplayTag & PhaseTag );

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

/**
 * Imported from Lyra
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGamePhaseSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    void StartPhase( TSubclassOf< UGBFGamePhaseAbility > phase_ability, FGBFGamePhaseDelegate phase_ended_callback = FGBFGamePhaseDelegate() );

    // TODO Return a handle so folks can delete these.  They will just grow until the world resets.
    // TODO Should we just occasionally clean these observers up?  It's not as if everyone will properly unhook them even if there is a handle.
    void WhenPhaseStartsOrIsActive( FGameplayTag phase_tag, EPhaseTagMatchType match_type, const FGBFGamePhaseTagDelegate & when_phase_active );
    void WhenPhaseEnds( FGameplayTag phase_tag, EPhaseTagMatchType match_type, const FGBFGamePhaseTagDelegate & when_phase_end );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = false, meta = ( AutoCreateRefTerm = "phase_tag" ) )
    bool IsPhaseActive( const FGameplayTag & phase_tag ) const;

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
    void DumpActivePhases( FOutputDevice & output_device );
#endif

protected:
    bool DoesSupportWorldType( const EWorldType::Type world_type ) const override;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = ( DisplayName = "Start Phase", AutoCreateRefTerm = "phase_ended_delegate" ) )
    void K2_StartPhase( TSubclassOf< UGBFGamePhaseAbility > phase_ability, const FGBFGamePhaseDynamicDelegate & phase_ended_delegate );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = ( DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "when_phase_active" ) )
    void K2_WhenPhaseStartsOrIsActive( FGameplayTag phase_tag, EPhaseTagMatchType match_type, FGBFGamePhaseTagDynamicDelegate when_phase_active );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Game Phase", meta = ( DisplayName = "When Phase Ends", AutoCreateRefTerm = "when_phase_end" ) )
    void K2_WhenPhaseEnds( FGameplayTag phase_tag, EPhaseTagMatchType match_type, FGBFGamePhaseTagDynamicDelegate when_phase_end );

    void OnBeginPhase( const UGBFGamePhaseAbility * phase_ability, const FGameplayAbilitySpecHandle phase_ability_handle );
    void OnEndPhase( const UGBFGamePhaseAbility * phase_ability, const FGameplayAbilitySpecHandle phase_ability_handle );

private:
    void GetActivePhases( TArray< FGameplayAbilitySpec * > & active_phases, UGASExtAbilitySystemComponent * asc ) const;

    struct FGBFGamePhaseEntry
    {
        FGameplayTag PhaseTag;
        FGBFGamePhaseDelegate PhaseEndedCallback;
    };

    TMap< FGameplayAbilitySpecHandle, FGBFGamePhaseEntry > ActivePhaseMap;

    struct FPhaseObserver
    {
        bool IsMatch( const FGameplayTag & compare_phase_tag ) const;

        FGameplayTag PhaseTag;
        EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
        FGBFGamePhaseTagDelegate PhaseCallback;
    };

    TArray< FPhaseObserver > PhaseStartObservers;
    TArray< FPhaseObserver > PhaseEndObservers;

    friend class UGBFGamePhaseAbility;
};
