#include "Interaction/Components/GBFInteractionDetectionComponent.h"

#include "Interaction/GBFInteractionStatics.h"

#include <Components/SphereComponent.h>

UGBFInteractionDetectionComponent::UGBFInteractionDetectionComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    DetectionRadius( 32.0f ),
    DetectionOffset( 0.0f )
{
}

void UGBFInteractionDetectionComponent::BeginPlay()
{
    Super::BeginPlay();

    SphereComponent = NewObject< USphereComponent >( GetOwner() );
    SphereComponent->RegisterComponent();
    SphereComponent->AttachToComponent( GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale );
    SphereComponent->SetCollisionProfileName( DetectionCollisionProfileName );
    SphereComponent->SetSphereRadius( DetectionRadius );
    SphereComponent->OnComponentBeginOverlap.AddDynamic( this, &ThisClass::OnComponentBeginOverlap );
    SphereComponent->OnComponentEndOverlap.AddDynamic( this, &ThisClass::OnComponentEndOverlap );
}

void UGBFInteractionDetectionComponent::EndPlay( const EEndPlayReason::Type end_play_reason )
{
    SphereComponent->UnregisterComponent();
    SphereComponent = nullptr;

    Super::EndPlay( end_play_reason );
}

void UGBFInteractionDetectionComponent::OnComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index, bool from_sweep, const FHitResult & sweep_result )
{
    TArray< TScriptInterface< IGBFInteractableTarget > > interactable_targets;
    UGBFInteractionStatics::GetInteractableTargetsFromActor( other_actor, interactable_targets );
}

void UGBFInteractionDetectionComponent::OnComponentEndOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index )
{
}