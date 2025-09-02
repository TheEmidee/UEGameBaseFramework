#include "GameFramework/GBFPlayerState.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

AGBFPlayerState::AGBFPlayerState(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // AbilitySystemComponent needs to be updated at a high frequency.
    SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* AGBFPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AGBFPlayerState::AddStatTagStack(const FGameplayTag Tag, const int32 StackCount)
{
    StatTags.AddStack(Tag, StackCount);
}

void AGBFPlayerState::RemoveStatTagStack(const FGameplayTag Tag, const int32 StackCount)
{
    StatTags.RemoveStack(Tag, StackCount);
}

void AGBFPlayerState::ResetStatTagStack(const FGameplayTag Tag)
{
    StatTags.ResetStack(Tag);
}

int32 AGBFPlayerState::GetStatTagStackCount(const FGameplayTag Tag) const
{
    return StatTags.GetStackCount(Tag);
}

bool AGBFPlayerState::HasStatTag(const FGameplayTag Tag) const
{
    return StatTags.ContainsTag(Tag);
}

void AGBFPlayerState::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check(AbilitySystemComponent != nullptr);
    AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
}

void AGBFPlayerState::OverrideWith(APlayerState* PlayerState)
{
    Super::OverrideWith(PlayerState);

    if (auto* GBFPlayerState = Cast<AGBFPlayerState>(PlayerState))
    {
        ConnectionOptions = GBFPlayerState->ConnectionOptions;
    }
}

void AGBFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams Params;
    Params.bIsPushBased = true;

    DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StatTags, Params);
}

void AGBFPlayerState::CopyProperties(APlayerState* PlayerState)
{
    Super::CopyProperties(PlayerState);

    if (auto* gbf_player_state = Cast<AGBFPlayerState>(PlayerState))
    {
        gbf_player_state->SetConnectionOptions(ConnectionOptions);
    }
}
