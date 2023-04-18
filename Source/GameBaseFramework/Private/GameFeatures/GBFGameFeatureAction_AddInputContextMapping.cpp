#include "GameFeatures/GBFGameFeatureAction_AddInputContextMapping.h"

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

#include "Characters/Components/GBFHeroComponent.h"

#include <Components/GameFrameworkComponentManager.h>
#include <Engine/LocalPlayer.h>
#include <EnhancedInputSubsystems.h>
#include <GameFramework/PlayerController.h>

#define LOCTEXT_NAMESPACE "UGFEGameFeatureAction_AddInputContextMapping"

void UGFEGameFeatureAction_AddInputContextMapping::OnGameFeatureActivating( FGameFeatureActivatingContext & context )
{
    if ( auto & active_data = ContextData.FindOrAdd( context );
         !ensure( active_data.ExtensionRequestHandles.IsEmpty() ) ||
         !ensure( active_data.ControllersAddedTo.IsEmpty() ) )
    {
        Reset( active_data );
    }
    Super::OnGameFeatureActivating( context );
}

void UGFEGameFeatureAction_AddInputContextMapping::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );

    if ( auto * active_data = ContextData.Find( context );
         ensure( active_data ) )
    {
        Reset( *active_data );
    }
}

#if WITH_EDITOR
EDataValidationResult UGFEGameFeatureAction_AddInputContextMapping::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .CustomValidation< TArray< FInputMappingContextAndPriority > >( InputMappings, []( TArray< FText > & errors, TArray< FInputMappingContextAndPriority > input_mappings ) {
            auto entry_index = 0;

            for ( const auto & input_mapping : input_mappings )
            {
                if ( input_mapping.InputMapping.IsNull() )
                {
                    errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullInputMapping", "Null InputMapping at index {0} in InputMappings" ), FText::AsNumber( entry_index ) ) );
                }

                ++entry_index;
            }
        } )
        .Result();
}
#endif

void UGFEGameFeatureAction_AddInputContextMapping::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    const auto * world = world_context.World();
    const auto game_instance = world_context.OwningGameInstance;
    auto & [ ExtensionRequestHandles, ControllersAddedTo ] = ContextData.FindOrAdd( change_context );

    if ( game_instance != nullptr && world != nullptr && world->IsGameWorld() )
    {
        if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
        {
            const auto add_abilities_delegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject( this, &ThisClass::HandleControllerExtension, change_context );
            const auto extension_request_handle = component_manager->AddExtensionHandler( APlayerController::StaticClass(), add_abilities_delegate );

            ExtensionRequestHandles.Add( extension_request_handle );
        }
    }
}

void UGFEGameFeatureAction_AddInputContextMapping::Reset( FPerContextData & active_data )
{
    active_data.ExtensionRequestHandles.Empty();

    while ( !active_data.ControllersAddedTo.IsEmpty() )
    {
        if ( auto controller_ptr = active_data.ControllersAddedTo.Top();
             controller_ptr.IsValid() )
        {
            RemoveInputMapping( controller_ptr.Get(), active_data );
        }
        else
        {
            active_data.ControllersAddedTo.Pop();
        }
    }
}

void UGFEGameFeatureAction_AddInputContextMapping::HandleControllerExtension( AActor * actor, FName event_name, const FGameFeatureStateChangeContext change_context )
{
    auto * pc = CastChecked< APlayerController >( actor );
    auto & active_data = ContextData.FindOrAdd( change_context );

    // TODO Why does this code mix and match controllers and local players? ControllersAddedTo is never modified
    if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionRemoved || event_name == UGameFrameworkComponentManager::NAME_ReceiverRemoved )
    {
        RemoveInputMapping( pc, active_data );
    }
    else if ( ( event_name == UGameFrameworkComponentManager::NAME_ExtensionAdded ) || ( event_name == UGBFHeroComponent::NAME_BindInputsNow ) )
    {
        AddInputMappingForPlayer( pc->GetLocalPlayer(), active_data );
    }
}

void UGFEGameFeatureAction_AddInputContextMapping::AddInputMappingForPlayer( UPlayer * player, FPerContextData & active_data )
{
    if ( const auto * lp = Cast< ULocalPlayer >( player ) )
    {
        if ( auto * input_system = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
        {
            for ( const auto & entry : InputMappings )
            {
                if ( const auto * imc = entry.InputMapping.Get() )
                {
                    input_system->AddMappingContext( imc, entry.Priority );
                }
            }
        }
        else
        {
            UE_LOG( LogGameFeatures, Error, TEXT( "Failed to find `UEnhancedInputLocalPlayerSubsystem` for local player. Input mappings will not be added. Make sure you're set to use the EnhancedInput system via config file." ) );
        }
    }
}

void UGFEGameFeatureAction_AddInputContextMapping::RemoveInputMapping( APlayerController * player_controller, FPerContextData & active_data )
{
    if ( const auto * lp = player_controller->GetLocalPlayer() )
    {
        if ( auto * input_system = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
        {
            for ( const auto & entry : InputMappings )
            {
                if ( const auto * imc = entry.InputMapping.Get() )
                {
                    input_system->RemoveMappingContext( imc );
                }
            }
        }
    }

    active_data.ControllersAddedTo.Remove( player_controller );
}

#undef LOCTEXT_NAMESPACE