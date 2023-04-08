#include "Characters/Components/GBFPawnComponent.h"

#include "GBFTags.h"

bool UGBFPawnComponent::IsPawnComponentReadyToInitialize() const
{
    // :TODO: Remove this function
    ensure( false );
    return true;
}

void UGBFPawnComponent::CheckDefaultInitialization()
{
    static const TArray< FGameplayTag > StateChain = { GBFTag_InitState_Spawned, GBFTag_InitState_DataAvailable, GBFTag_InitState_DataInitialized, GBFTag_InitState_GameplayReady };

    // This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
    ContinueInitStateChain( StateChain );
}

void UGBFPawnComponent::OnRegister()
{
    Super::OnRegister();

    // Register with the init state system early, this will only work if this is a game world
    RegisterInitStateFeature();
}

void UGBFPawnComponent::BeginPlay()
{
    Super::BeginPlay();

    BindToRequiredOnActorInitStateChanged();

    // Notifies state manager that we have spawned, then try rest of default initialization
    ensure( TryToChangeInitState( GBFTag_InitState_Spawned ) );
    CheckDefaultInitialization();
}

void UGBFPawnComponent::EndPlay( const EEndPlayReason::Type end_play_reason )
{
    UnregisterInitStateFeature();

    Super::EndPlay( end_play_reason );
}

void UGBFPawnComponent::BindToRequiredOnActorInitStateChanged()
{
}
