#include "Characters/Components/GBFHeroComponent.h"

#include "Camera/GBFCameraComponent.h"
#include "Camera/GBFCameraMode.h"
#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "Characters/GBFPawnData.h"
#include "Components/GASExtAbilitySystemComponent.h"
#include "GBFLog.h"
#include "GameFramework/GBFPlayerController.h"
#include "GameFramework/GBFPlayerState.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <GameFramework/Controller.h>
#include <Logging/MessageLog.h>
#include <Misc/UObjectToken.h>

UGBFHeroComponent::UGBFHeroComponent()
{
    bPawnHasInitialized = false;
    bReadyToBindInputs = false;
    AbilityCameraMode = nullptr;
}

UGBFHeroComponent * UGBFHeroComponent::FindHeroComponent( const AActor * actor )
{
    return actor ? actor->FindComponentByClass< UGBFHeroComponent >() : nullptr;
}

void UGBFHeroComponent::SetAbilityCameraMode( TSubclassOf< UGBFCameraMode > camera_mode, const FGameplayAbilitySpecHandle & owning_spec_handle )
{
    if ( camera_mode )
    {
        AbilityCameraMode = camera_mode;
        AbilityCameraModeOwningSpecHandle = owning_spec_handle;
    }
}

void UGBFHeroComponent::ClearAbilityCameraMode( const FGameplayAbilitySpecHandle & owning_spec_handle )
{
    if ( AbilityCameraModeOwningSpecHandle == owning_spec_handle )
    {
        AbilityCameraMode = nullptr;
        AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
    }
}

void UGBFHeroComponent::OnRegister()
{
    Super::OnRegister();

    if ( const auto * pawn = GetPawn< APawn >() )
    {
        if ( auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
        {
            OnPawnReadyToInitializeDelegate = FSimpleMulticastDelegate::FDelegate::CreateUObject( this, &ThisClass::OnPawnReadyToInitialize );
            pawn_ext_comp->OnPawnReadyToInitialize_RegisterAndCall( OnPawnReadyToInitializeDelegate );
        }
    }
    else
    {
        UE_LOG( LogGBF, Error, TEXT( "[UGBFHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint." ) );

#if WITH_EDITOR
        if ( GIsEditor )
        {
            static const FText Message = NSLOCTEXT( "UGBFHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!" );
            static const FName HeroMessageLogName = TEXT( "UGBFHeroComponent" );

            FMessageLog( HeroMessageLogName ).Error()->AddToken( FUObjectToken::Create( this, FText::FromString( GetNameSafe( this ) ) ) )->AddToken( FTextToken::Create( Message ) );
            FMessageLog( HeroMessageLogName ).Open();
        }
#endif
    }
}

void UGBFHeroComponent::OnUnregister()
{
    if ( const auto * pawn = GetPawn< APawn >() )
    {
        if ( auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
        {
            pawn_ext_comp->UninitializeAbilitySystem();
            pawn_ext_comp->OnPawnReadyToInitialize_UnRegister( OnPawnReadyToInitializeDelegate );
        }
    }

    bPawnHasInitialized = false;

    Super::OnUnregister();
}

bool UGBFHeroComponent::IsPawnComponentReadyToInitialize() const
{
    // The player state is required.
    if ( !GetPlayerState< AGBFPlayerState >() )
    {
        return false;
    }

    const auto * pawn = GetPawn< APawn >();

    // A pawn is required.
    if ( pawn == nullptr )
    {
        return false;
    }

    // If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
    if ( pawn->GetLocalRole() != ROLE_SimulatedProxy )
    {
        const auto * controller = GetController< AController >();

        const auto has_controller_paired_with_ps = ( controller != nullptr ) &&
                                                   ( controller->PlayerState != nullptr ) &&
                                                   ( controller->PlayerState->GetOwner() == controller );

        if ( !has_controller_paired_with_ps )
        {
            return false;
        }
    }

    const auto is_locally_controlled = pawn->IsLocallyControlled();
    const auto is_bot = pawn->IsBotControlled();

    if ( is_locally_controlled && !is_bot )
    {
        // The input component is required when locally controlled.
        if ( !pawn->InputComponent )
        {
            return false;
        }
    }

    return true;
}

void UGBFHeroComponent::OnPawnReadyToInitialize()
{
    if ( !ensure( !bPawnHasInitialized ) )
    {
        // Don't initialize twice
        return;
    }

    const auto * pawn = GetPawn< APawn >();
    if ( !pawn )
    {
        return;
    }

    const auto is_locally_controlled = pawn->IsLocallyControlled();

    auto * player_state = GetPlayerState< AGBFPlayerState >();
    check( player_state != nullptr );

    const UGBFPawnData * pawn_data = nullptr;

    if ( auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
    {
        pawn_data = pawn_ext_comp->GetPawnData< UGBFPawnData >();

        // The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
        // The ability system component and attribute sets live on the player state.
        pawn_ext_comp->InitializeAbilitySystem( player_state->GetGASExtAbilitySystemComponent(), player_state );
    }

    if ( pawn_data != nullptr )
    {
        player_state->SetPawnData( pawn_data );
    }

    if ( auto * pc = GetController< AGBFPlayerController >() )
    {
        if ( pawn->InputComponent != nullptr )
        {
            InitializePlayerInput( pawn->InputComponent );
        }

        if ( is_locally_controlled && pawn_data != nullptr )
        {
            if ( auto * player_camera_manager = pc->PlayerCameraManager.Get() )
            {
                for ( const auto & modifier : pawn_data->CameraModifiers )
                {
                    player_camera_manager->AddNewCameraModifier( modifier );
                }
            }

            if ( auto * camera_component = UGBFCameraComponent::FindCameraComponent( pawn ) )
            {
                camera_component->DetermineCameraModeDelegate.BindUObject( this, &ThisClass::DetermineCameraMode );
            }
        }
    }

    bPawnHasInitialized = true;
}

void UGBFHeroComponent::InitializePlayerInput( UInputComponent * player_input_component )
{
}

TSubclassOf< UGBFCameraMode > UGBFHeroComponent::DetermineCameraMode() const
{
    if ( AbilityCameraMode )
    {
        return AbilityCameraMode;
    }

    const auto * pawn = GetPawn< APawn >();
    if ( !pawn )
    {
        return nullptr;
    }

    if ( const auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
    {
        if ( const auto * pawn_data = pawn_ext_comp->GetPawnData< UGBFPawnData >() )
        {
            return pawn_data->DefaultCameraMode;
        }
    }

    return nullptr;
}