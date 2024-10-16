#include "GameFeatures/GBFGameFeatureAction_AddWidget.h"

#include "Engine/GBFAssetManager.h"
#include "Engine/GBFHUD.h"

#include <Blueprint/GameViewportSubsystem.h>
#include <CommonActivatableWidget.h>
#include <CommonUIExtensions.h>
#include <Components/GameFrameworkComponentManager.h>
#include <Engine/GameInstance.h>
#include <Engine/LocalPlayer.h>
#include <Engine/World.h>
#include <GameFeaturesSubsystemSettings.h>
#include <GameFramework/PlayerController.h>

#if WITH_EDITOR
#include <Misc/DataValidation.h>
#endif

#define LOCTEXT_NAMESPACE "GameBaseFrameworkGameFeatures"

void UGBFGameFeatureAction_AddWidget::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );

    auto * active_data = ContextData.Find( context );
    if ( ensure( active_data != nullptr ) )
    {
        Reset( *active_data );
    }
}

#if WITH_EDITORONLY_DATA
void UGBFGameFeatureAction_AddWidget::AddAdditionalAssetBundleData( FAssetBundleData & asset_bundle_data )
{
    for ( const auto & entry : Widgets )
    {
        asset_bundle_data.AddBundleAsset( UGameFeaturesSubsystemSettings::LoadStateClient, entry.WidgetClass.ToSoftObjectPath().GetAssetPath() );
    }
}
#endif

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_AddWidget::IsDataValid( FDataValidationContext & context ) const
{
    auto result = CombineDataValidationResults( Super::IsDataValid( context ), EDataValidationResult::Valid );
    {
        auto entry_index = 0;
        for ( const auto & [ layout_class, layer_id, controller_id ] : Layout )
        {
            if ( layout_class.IsNull() )
            {
                result = EDataValidationResult::Invalid;
                context.AddError( FText::Format( LOCTEXT( "LayoutHasNullClass", "Null WidgetClass at index {0} in Layout" ), FText::AsNumber( entry_index ) ) );
            }

            if ( !layer_id.IsValid() )
            {
                result = EDataValidationResult::Invalid;
                context.AddError( FText::Format( LOCTEXT( "LayoutHasNoTag", "LayerID is not set at index {0} in Widgets" ), FText::AsNumber( entry_index ) ) );
            }

            ++entry_index;
        }
    }

    {
        auto entry_index = 0;
        for ( const auto & [ widget_class, slot_id, controller_id ] : Widgets )
        {
            if ( widget_class.IsNull() )
            {
                result = EDataValidationResult::Invalid;
                context.AddError( FText::Format( LOCTEXT( "EntryHasNullClass", "Null WidgetClass at index {0} in Widgets" ), FText::AsNumber( entry_index ) ) );
            }

            if ( !slot_id.IsValid() )
            {
                result = EDataValidationResult::Invalid;
                context.AddError( FText::Format( LOCTEXT( "EntryHasNoTag", "SlotID is not set at index {0} in Widgets" ), FText::AsNumber( entry_index ) ) );
            }
            ++entry_index;
        }
    }

    return result;
}

#endif

void UGBFGameFeatureAction_AddWidget::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    const auto * world = world_context.World();
    const auto game_instance = world_context.OwningGameInstance;
    auto & active_data = ContextData.FindOrAdd( change_context );

    if ( game_instance != nullptr && world != nullptr && world->IsGameWorld() )
    {
        if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
        {
            const TSoftClassPtr< AActor > hud_actor_class = AGBFHUD::StaticClass();

            const auto extension_request_handle = component_manager->AddExtensionHandler(
                hud_actor_class,
                UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject( this, &ThisClass::HandleActorExtension, change_context ) );

            active_data.ComponentRequests.Add( extension_request_handle );
        }
    }
}

void UGBFGameFeatureAction_AddWidget::Reset( FPerContextData & active_data )
{
    active_data.ComponentRequests.Empty();

    for ( auto & pair : active_data.ActorData )
    {
        for ( auto & handle : pair.Value.ExtensionHandles )
        {
            handle.Unregister();
        }
    }
    active_data.ActorData.Empty();
}

void UGBFGameFeatureAction_AddWidget::HandleActorExtension( AActor * actor, const FName event_name, const FGameFeatureStateChangeContext change_context )
{
    auto & active_data = ContextData.FindOrAdd( change_context );
    if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionRemoved || event_name == UGameFrameworkComponentManager::NAME_ReceiverRemoved )
    {
        RemoveWidgets( actor, active_data );
    }
    else if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionAdded || event_name == UGameFrameworkComponentManager::NAME_GameActorReady )
    {
        AddWidgets( actor, active_data );
    }
}

void UGBFGameFeatureAction_AddWidget::AddWidgets( AActor * actor, FPerContextData & active_data )
{
    const auto * hud = CastChecked< AGBFHUD >( actor );

    if ( hud->GetOwningPlayerController() == nullptr )
    {
        return;
    }

    auto & asset_manager = UGBFAssetManager::Get();

    if ( auto * local_player = Cast< ULocalPlayer >( hud->GetOwningPlayerController()->Player ) )
    {
        auto & per_actor_data = active_data.ActorData.FindOrAdd( hud );

        for ( const auto & entry : Layout )
        {
            if ( entry.ControllerId == -1 || local_player->GetControllerId() == entry.ControllerId )
            {
                if ( const auto concrete_widget_class = asset_manager.GetSubclass( entry.LayoutClass ) )
                {
                    per_actor_data.LayoutsAdded.Add( UCommonUIExtensions::PushContentToLayer_ForPlayer( local_player, entry.LayerID, concrete_widget_class ) );
                }
            }
        }

        auto * extension_subsystem = hud->GetWorld()->GetSubsystem< UUIExtensionSubsystem >();
        for ( const auto & entry : Widgets )
        {
            if ( entry.ControllerId == -1 || local_player->GetControllerId() == entry.ControllerId )
            {
                if ( const auto widget_class = asset_manager.GetSubclass( entry.WidgetClass ) )
                {
                    per_actor_data.ExtensionHandles.Add( extension_subsystem->RegisterExtensionAsWidgetForContext( entry.SlotID, local_player, widget_class, -1 ) );
                }
            }
        }

        if ( local_player->GetControllerId() == 0 )
        {
            for ( const auto & overlay : Overlay )
            {
                if ( const auto widget_class = asset_manager.GetSubclass( overlay.WidgetClass ) )
                {
                    auto * widget = CreateWidget( hud->GetOwningPlayerController(), widget_class );
                    active_data.OverlayWidgets.Add( widget );

                    widget->AddToViewport();
                }
            }
        }
    }
}

void UGBFGameFeatureAction_AddWidget::RemoveWidgets( AActor * actor, FPerContextData & active_data )
{
    const auto * hud = CastChecked< AGBFHUD >( actor );

    // Only unregister if this is the same HUD actor that was registered, there can be multiple active at once on the client

    if ( auto * actor_data = active_data.ActorData.Find( hud ) )
    {
        for ( auto & added_layout : actor_data->LayoutsAdded )
        {
            if ( added_layout.IsValid() )
            {
                added_layout->DeactivateWidget();
            }
        }
        for ( auto & handle : actor_data->ExtensionHandles )
        {
            handle.Unregister();
        }
        active_data.ActorData.Remove( hud );

        for ( const auto overlay_widget : active_data.OverlayWidgets )
        {
            overlay_widget->RemoveFromParent();
        }

        active_data.OverlayWidgets.Reset();
    }
}

#undef LOCTEXT_NAMESPACE
