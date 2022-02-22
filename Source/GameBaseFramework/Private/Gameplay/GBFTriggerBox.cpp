#include "Gameplay/GBFTriggerBox.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

#if WITH_EDITORONLY_DATA
#include <Components/BillboardComponent.h>
#endif

#include <Components/ShapeComponent.h>

AGBFTriggerBox::AGBFTriggerBox()
{
    TriggerManagerComponent = CreateDefaultSubobject< UGBFTriggerManagerComponent >( TEXT( "TriggerManagerComponent" ) );
    TriggerManagerComponent->SetObservedCollisionComponent( GetCollisionComponent() );

    GetCollisionComponent()->bHiddenInGame = true;

#if WITH_EDITORONLY_DATA
    if ( auto * sprite_component = GetSpriteComponent() )
    {
        sprite_component->bHiddenInGame = true;
    }
#endif
}

void AGBFTriggerBox::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &AGBFTriggerBox::OnTriggerBoxActivated );
}

void AGBFTriggerBox::OnTriggerBoxActivated_Implementation( AActor * /*activator*/ )
{
}
