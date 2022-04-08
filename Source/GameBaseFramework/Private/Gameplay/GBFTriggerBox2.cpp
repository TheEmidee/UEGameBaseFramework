#include "Gameplay/GBFTriggerBox2.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

#include <Components/BoxComponent.h>
#include <UObject/ConstructorHelpers.h>

#if WITH_EDITORONLY_DATA
#include <Components/BillboardComponent.h>
#endif

AGBFTriggerBox2::AGBFTriggerBox2()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject< USceneComponent >( TEXT( "RootComponent" ) );
    RootComponent = RootSceneComponent;

    BoxComponent = CreateDefaultSubobject< UBoxComponent >( TEXT( "CollisionComp" ) );

    BoxComponent->SetupAttachment( RootSceneComponent );
    BoxComponent->bHiddenInGame = false;

    TriggerManagerComponent = CreateDefaultSubobject< UGBFTriggerManagerComponent >( TEXT( "TriggerManagerComponent" ) );
    TriggerManagerComponent->SetObservedCollisionComponent( BoxComponent );

    BoxComponent->bHiddenInGame = true;

#if WITH_EDITORONLY_DATA
    SpriteComponent = CreateEditorOnlyDefaultSubobject< UBillboardComponent >( TEXT( "Sprite" ) );
    if ( SpriteComponent )
    {
        // Structure to hold one-time initialization
        struct FConstructorStatics
        {
            ConstructorHelpers::FObjectFinderOptional< UTexture2D > TriggerTextureObject;
            FName ID_Triggers;
            FText NAME_Triggers;
            FConstructorStatics() :
                TriggerTextureObject( TEXT( "/Engine/EditorResources/S_Trigger" ) ),
                ID_Triggers( TEXT( "Triggers" ) ),
                NAME_Triggers( NSLOCTEXT( "SpriteCategory", "Triggers", "Triggers" ) )
            {
            }
        };
        static FConstructorStatics ConstructorStatics;

        SpriteComponent->Sprite = ConstructorStatics.TriggerTextureObject.Get();
        SpriteComponent->SetRelativeScale3D( FVector( 0.5f, 0.5f, 0.5f ) );
        SpriteComponent->bHiddenInGame = false;
        SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Triggers;
        SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Triggers;
        SpriteComponent->bIsScreenSizeScaled = true;
    }
#endif
}

void AGBFTriggerBox2::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &AGBFTriggerBox2::OnTriggerBoxActivated );
}

void AGBFTriggerBox2::Multicast_ActivateTrigger_Implementation( const bool reset )
{
    TriggerManagerComponent->Activate( reset );
}

void AGBFTriggerBox2::Multicast_DeactivateTrigger_Implementation()
{
    TriggerManagerComponent->Deactivate();
}

void AGBFTriggerBox2::OnTriggerBoxActivated_Implementation( AActor * /*activator*/ )
{
}
