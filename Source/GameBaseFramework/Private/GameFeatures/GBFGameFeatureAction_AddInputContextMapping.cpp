#include "GameFeatures/GBFGameFeatureAction_AddInputContextMapping.h"

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

#include "Characters/Components/GBFHeroComponent.h"
#include "Engine/GBFAssetManager.h"

#include <Components/GameFrameworkComponentManager.h>
#include <Engine/LocalPlayer.h>
#include <EnhancedInputSubsystems.h>
#include <GameFramework/PlayerController.h>
#include <UserSettings/EnhancedInputUserSettings.h>

#define LOCTEXT_NAMESPACE "UGFEGameFeatureAction_AddInputContextMapping"

void UGBFGameFeatureAction_AddInputContextMapping::OnGameFeatureRegistering()
{
    Super::OnGameFeatureRegistering();

    RegisterInputMappingContexts();
}

void UGBFGameFeatureAction_AddInputContextMapping::OnGameFeatureActivating( FGameFeatureActivatingContext & context )
{
    if ( auto & active_data = ContextData.FindOrAdd( context );
         !ensure( active_data.ExtensionRequestHandles.IsEmpty() ) ||
         !ensure( active_data.ControllersAddedTo.IsEmpty() ) )
    {
        Reset( active_data );
    }
    Super::OnGameFeatureActivating( context );
}

void UGBFGameFeatureAction_AddInputContextMapping::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );

    if ( auto * active_data = ContextData.Find( context );
         ensure( active_data != nullptr ) )
    {
        Reset( *active_data );
    }
}

void UGBFGameFeatureAction_AddInputContextMapping::OnGameFeatureUnregistering()
{
    Super::OnGameFeatureUnregistering();

    UnregisterInputMappingContexts();
}

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_AddInputContextMapping::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .CustomValidation< TArray< FGBFInputMappingContextAndPriority > >( InputMappings, []( TArray< FText > & errors, TArray< FGBFInputMappingContextAndPriority > input_mappings ) {
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

void UGBFGameFeatureAction_AddInputContextMapping::RegisterInputMappingContexts()
{
    RegisterInputContextMappingsForGameInstanceHandle = FWorldDelegates::OnStartGameInstance.AddUObject( this, &UGBFGameFeatureAction_AddInputContextMapping::RegisterInputContextMappingsForGameInstance );

    const auto & world_contexts = GEngine->GetWorldContexts();
    for ( auto world_context_iterator = world_contexts.CreateConstIterator(); world_context_iterator; ++world_context_iterator )
    {
        RegisterInputContextMappingsForGameInstance( world_context_iterator->OwningGameInstance );
    }
}

void UGBFGameFeatureAction_AddInputContextMapping::RegisterInputContextMappingsForGameInstance( UGameInstance * game_instance )
{
    if ( game_instance != nullptr && !game_instance->OnLocalPlayerAddedEvent.IsBoundToObject( this ) )
    {
        game_instance->OnLocalPlayerAddedEvent.AddUObject( this, &UGBFGameFeatureAction_AddInputContextMapping::RegisterInputMappingContextsForLocalPlayer );
        game_instance->OnLocalPlayerRemovedEvent.AddUObject( this, &UGBFGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContextsForLocalPlayer );

        for ( auto local_player_iterator = game_instance->GetLocalPlayerIterator(); local_player_iterator; ++local_player_iterator )
        {
            RegisterInputMappingContextsForLocalPlayer( *local_player_iterator );
        }
    }
}

void UGBFGameFeatureAction_AddInputContextMapping::RegisterInputMappingContextsForLocalPlayer( ULocalPlayer * local_player )
{
    if ( ensure( local_player ) )
    {
        auto & asset_manager = UGBFAssetManager::Get();

        if ( const auto * ei_subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem >( local_player ) )
        {
            if ( auto * settings = ei_subsystem->GetUserSettings() )
            {
                for ( const auto & entry : InputMappings )
                {
                    // Skip entries that don't want to be registered
                    if ( !entry.bRegisterWithSettings )
                    {
                        continue;
                    }

                    // Register this IMC with the settings!
                    if ( const auto * imc = asset_manager.GetAsset( entry.InputMapping ) )
                    {
                        settings->RegisterInputMappingContext( imc );
                    }
                }
            }
        }
    }
}

void UGBFGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContexts()
{
    FWorldDelegates::OnStartGameInstance.Remove( RegisterInputContextMappingsForGameInstanceHandle );
    RegisterInputContextMappingsForGameInstanceHandle.Reset();

    const auto & world_contexts = GEngine->GetWorldContexts();
    for ( auto world_context_iterator = world_contexts.CreateConstIterator(); world_context_iterator; ++world_context_iterator )
    {
        UnregisterInputContextMappingsForGameInstance( world_context_iterator->OwningGameInstance );
    }
}

void UGBFGameFeatureAction_AddInputContextMapping::UnregisterInputContextMappingsForGameInstance( UGameInstance * game_instance )
{
    if ( game_instance != nullptr )
    {
        game_instance->OnLocalPlayerAddedEvent.RemoveAll( this );
        game_instance->OnLocalPlayerRemovedEvent.RemoveAll( this );

        for ( auto local_player_iterator = game_instance->GetLocalPlayerIterator(); local_player_iterator; ++local_player_iterator )
        {
            UnregisterInputMappingContextsForLocalPlayer( *local_player_iterator );
        }
    }
}

void UGBFGameFeatureAction_AddInputContextMapping::UnregisterInputMappingContextsForLocalPlayer( ULocalPlayer * local_player )
{
    if ( ensure( local_player ) )
    {
        if ( const auto * ei_subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem >( local_player ) )
        {
            if ( auto * settings = ei_subsystem->GetUserSettings() )
            {
                for ( const auto & entry : InputMappings )
                {
                    // Skip entries that don't want to be registered
                    if ( !entry.bRegisterWithSettings )
                    {
                        continue;
                    }

                    // Register this IMC with the settings!
                    if ( const auto * imc = entry.InputMapping.Get() )
                    {
                        settings->UnregisterInputMappingContext( imc );
                    }
                }
            }
        }
    }
}

void UGBFGameFeatureAction_AddInputContextMapping::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
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

void UGBFGameFeatureAction_AddInputContextMapping::Reset( FPerContextData & active_data )
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

void UGBFGameFeatureAction_AddInputContextMapping::HandleControllerExtension( AActor * actor, FName event_name, const FGameFeatureStateChangeContext change_context )
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

void UGBFGameFeatureAction_AddInputContextMapping::AddInputMappingForPlayer( UPlayer * player, FPerContextData & active_data )
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

void UGBFGameFeatureAction_AddInputContextMapping::RemoveInputMapping( APlayerController * player_controller, FPerContextData & active_data )
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