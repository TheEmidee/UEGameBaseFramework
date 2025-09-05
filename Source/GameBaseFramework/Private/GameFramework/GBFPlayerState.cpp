#include "GameFramework/GBFPlayerState.h"

#include "Net/UnrealNetwork.h"

AGBFPlayerState::AGBFPlayerState(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
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
