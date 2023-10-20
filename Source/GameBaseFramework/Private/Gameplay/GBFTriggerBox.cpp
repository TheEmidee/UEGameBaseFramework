#include "Gameplay/GBFTriggerBox.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

#include <Components/BoxComponent.h>
#include <UObject/ConstructorHelpers.h>

#if WITH_EDITORONLY_DATA
#include <Components/BillboardComponent.h>
#include <Engine/Texture2D.h>
#endif

AGBFTriggerBox::AGBFTriggerBox()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject< USceneComponent >( TEXT( "RootComponent" ) );
    RootSceneComponent->Mobility = EComponentMobility::Static;
    RootComponent = RootSceneComponent;

    BoxComponent = CreateDefaultSubobject< UBoxComponent >( TEXT( "CollisionComp" ) );
    BoxComponent->SetupAttachment( RootSceneComponent );
    BoxComponent->SetCollisionProfileName( "Trigger" );
    BoxComponent->bHiddenInGame = true;
    BoxComponent->Mobility = EComponentMobility::Static;

    TriggerManagerComponent = CreateDefaultSubobject< UGBFTriggerManagerComponent >( TEXT( "TriggerManagerComponent" ) );
    TriggerManagerComponent->SetObservedCollisionComponent( BoxComponent );

#if WITH_EDITORONLY_DATA
    SpriteComponent = CreateEditorOnlyDefaultSubobject< UBillboardComponent >( TEXT( "Sprite" ) );

    if ( SpriteComponent )
    {
        SpriteComponent->SetupAttachment( RootSceneComponent );

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
        SpriteComponent->bHiddenInGame = true;
        SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Triggers;
        SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Triggers;
        SpriteComponent->bIsScreenSizeScaled = true;
    }
#endif
}

void AGBFTriggerBox::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &AGBFTriggerBox::OnTriggerBoxActivated );
}

void AGBFTriggerBox::Multicast_ActivateTrigger_Implementation( const bool reset )
{
    TriggerManagerComponent->Activate( reset );
}

void AGBFTriggerBox::Multicast_DeactivateTrigger_Implementation()
{
    TriggerManagerComponent->Deactivate();
}

void AGBFTriggerBox::OnTriggerBoxActivated_Implementation( UGBFTriggerManagerComponent * /*component*/, AActor * /*activator*/ )
{
}
