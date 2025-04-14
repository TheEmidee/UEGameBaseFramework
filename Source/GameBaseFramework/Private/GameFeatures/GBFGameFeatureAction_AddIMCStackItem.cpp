#include "GameFeatures/GBFGameFeatureAction_AddIMCStackItem.h"

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

#include "Characters/Components/GBFHeroComponent.h"

#include <Components/GameFrameworkComponentManager.h>
#include <Engine/GameInstance.h>
#include <GameFramework/PlayerController.h>

#define LOCTEXT_NAMESPACE "UGFEGameFeatureAction_AddIMCStackItem"

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_AddIMCStackItem::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .NotNull( VALIDATOR_GET_PROPERTY( IMCStackItem ) )
        .Result();
}
#endif

void UGBFGameFeatureAction_AddIMCStackItem::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    const auto * world = world_context.World();
    const auto game_instance = world_context.OwningGameInstance;

    if ( game_instance != nullptr && world != nullptr && world->IsGameWorld() )
    {
        if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
        {
            const auto add_imc_stack_item_delegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject( this, &ThisClass::HandleControllerExtension, change_context );
            component_manager->AddExtensionHandler( APlayerController::StaticClass(), add_imc_stack_item_delegate );
        }
    }
}

void UGBFGameFeatureAction_AddIMCStackItem::HandleControllerExtension( AActor * actor, const FName event_name, const FGameFeatureStateChangeContext /*change_context*/ )
{
    const auto * player_controller = CastChecked< APlayerController >( actor );
    if ( player_controller == nullptr )
    {
        return;
    }

    const auto * pawn = player_controller->GetPawn();
    if ( pawn == nullptr )
    {
        return;
    }

    auto * hero_component = UGBFHeroComponent::FindHeroComponent( pawn );
    if ( hero_component == nullptr )
    {
        return;
    }

    if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionRemoved || event_name == UGameFrameworkComponentManager::NAME_ReceiverRemoved )
    {
        hero_component->RemoveIMCStackItem( IMCStackItem );
    }
    else if ( ( event_name == UGameFrameworkComponentManager::NAME_ExtensionAdded ) || ( event_name == UGBFHeroComponent::NAME_BindInputsNow ) )
    {
        hero_component->AddIMCStackItem( IMCStackItem );
    }
}

#undef LOCTEXT_NAMESPACE