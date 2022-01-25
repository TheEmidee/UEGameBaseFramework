#include "Gameplay/GBFTriggerBox.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

#include <Components/ShapeComponent.h>

AGBFTriggerBox::AGBFTriggerBox()
{
    TriggerManagerComponent = CreateDefaultSubobject< UGBFTriggerManagerComponent >( TEXT( "TriggerManagerComponent" ) );
    TriggerManagerComponent->SetObservedCollisionComponent( GetCollisionComponent() );

    GetCollisionComponent()->bHiddenInGame = true;
}

void AGBFTriggerBox::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &AGBFTriggerBox::OnTriggerBoxActivated );
}

void AGBFTriggerBox::OnTriggerBoxActivated_Implementation( AActor * /*activator*/ )
{
}
