#include "Characters/Components/GBFHeroComponent.h"

#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "Characters/GBFPawnData.h"
#include "Components/GASExtAbilitySystemComponent.h"
#include "Engine/GBFLocalPlayer.h"
#include "GBFLog.h"
#include "GBFTags.h"
#include "GameFramework/GBFPlayerController.h"
#include "GameFramework/GBFPlayerState.h"
#include "Input/GBFInputComponent.h"
#include "Input/GBFMappableConfigPair.h"

#include <AbilitySystemBlueprintLibrary.h>
#include <Components/GameFrameworkComponentManager.h>
#include <EnhancedInputComponent.h>
#include <EnhancedInputSubsystemInterface.h>
#include <EnhancedInputSubsystems.h>
#include <GameFramework/Controller.h>
#include <Logging/MessageLog.h>
#include <Misc/UObjectToken.h>

const FName UGBFHeroComponent::NAME_BindInputsNow( "BindInputsNow" );
const FName UGBFHeroComponent::NAME_ActorFeatureName( "HeroComponent" );

UGBFHeroComponent::UGBFHeroComponent()
{
    bReadyToBindInputs = false;
}

FName UGBFHeroComponent::GetFeatureName() const
{
    return NAME_ActorFeatureName;
}

bool UGBFHeroComponent::CanChangeInitState( UGameFrameworkComponentManager * manager, FGameplayTag current_state, FGameplayTag desired_state ) const
{
    check( manager != nullptr );

    auto * pawn = GetPawn< APawn >();
    const auto * ps = GetPlayerState< AGBFPlayerState >();

    if ( !current_state.IsValid() && desired_state == GBFTag_InitState_Spawned )
    {
        return pawn != nullptr;
    }
    if ( current_state == GBFTag_InitState_Spawned && desired_state == GBFTag_InitState_DataAvailable )
    {
        if ( ps == nullptr )
        {
            return false;
        }

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
            if ( const auto * pc = GetController< APlayerController >();
                 pawn->InputComponent == nullptr || pc == nullptr || pc->GetLocalPlayer() == nullptr )
            {
                return false;
            }
        }

        return true;
    }
    if ( current_state == GBFTag_InitState_DataAvailable && desired_state == GBFTag_InitState_DataInitialized )
    {
        return ps && manager->HasFeatureReachedInitState( pawn, UGBFPawnExtensionComponent::NAME_ActorFeatureName, GBFTag_InitState_DataInitialized );
    }
    if ( current_state == GBFTag_InitState_DataInitialized && desired_state == GBFTag_InitState_GameplayReady )
    {
        return true;
    }

    return false;
}

void UGBFHeroComponent::HandleChangeInitState( UGameFrameworkComponentManager * manager, FGameplayTag current_state, FGameplayTag desired_state )
{
    if ( current_state == GBFTag_InitState_DataAvailable && desired_state == GBFTag_InitState_DataInitialized )
    {
        const auto * pawn = GetPawn< APawn >();
        auto * player_state = GetPlayerState< AGBFPlayerState >();

        if ( !ensure( pawn != nullptr && player_state != nullptr ) )
        {
            return;
        }

        const auto is_locally_controlled = pawn->IsLocallyControlled();

        const UGBFPawnData * pawn_data = nullptr;

        if ( auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
        {
            pawn_data = pawn_ext_comp->GetPawnData< UGBFPawnData >();

            // The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
            // The ability system component and attribute sets live on the player state.
            pawn_ext_comp->InitializeAbilitySystem( player_state->GetGASExtAbilitySystemComponent(), player_state );
        }

        if ( const auto * pc = GetController< AGBFPlayerController >() )
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
            }
        }
    }
}

void UGBFHeroComponent::OnActorInitStateChanged( const FActorInitStateChangedParams & params )
{
    if ( params.FeatureName == UGBFPawnExtensionComponent::NAME_ActorFeatureName )
    {
        if ( params.FeatureState == GBFTag_InitState_DataInitialized )
        {
            // If the extension component says all all other components are initialized, try to progress to next state
            CheckDefaultInitialization();
        }
    }
}

void UGBFHeroComponent::AddAdditionalInputConfig( const UGBFInputConfig * input_config )
{
    const auto * pawn = GetPawn< APawn >();
    if ( !pawn )
    {
        return;
    }

    auto * input_component = pawn->FindComponentByClass< UGBFInputComponent >();
    check( input_component != nullptr );

    const APlayerController * pc = GetController< APlayerController >();
    check( pc != nullptr );

    const ULocalPlayer * lp = pc->GetLocalPlayer();
    check( lp != nullptr );

    const UEnhancedInputLocalPlayerSubsystem * subsystem = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >();
    check( subsystem );

    if ( UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) != nullptr )
    {
        TArray< uint32 > bind_handles;
        input_component->BindAbilityActions( input_config, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ bind_handles );
    }
}

void UGBFHeroComponent::RemoveAdditionalInputConfig( const UGBFInputConfig * input_config )
{
}

UGBFHeroComponent * UGBFHeroComponent::FindHeroComponent( const AActor * actor )
{
    return actor ? actor->FindComponentByClass< UGBFHeroComponent >() : nullptr;
}

void UGBFHeroComponent::OnRegister()
{
    Super::OnRegister();

    if ( GetPawn< APawn >() == nullptr )
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

void UGBFHeroComponent::BindToRequiredOnActorInitStateChanged()
{
    BindOnActorInitStateChanged( UGBFPawnExtensionComponent::NAME_ActorFeatureName, GBFTag_InitState_DataInitialized, false );
}

void UGBFHeroComponent::InitializePlayerInput( UInputComponent * player_input_component )
{
    check( player_input_component != nullptr );

    const APawn * pawn = GetPawn< APawn >();
    if ( pawn == nullptr )
    {
        return;
    }

    const APlayerController * pc = GetController< APlayerController >();
    check( pc != nullptr );

    const auto * lp = Cast< UGBFLocalPlayer >( pc->GetLocalPlayer() );
    check( lp != nullptr );

    auto * enhanced_input_local_player_subsystem = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >();
    check( enhanced_input_local_player_subsystem != nullptr );

    enhanced_input_local_player_subsystem->ClearAllMappings();

    if ( const auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
    {
        if ( const auto * pawn_data = pawn_ext_comp->GetPawnData< UGBFPawnData >() )
        {
            auto * input_component = CastChecked< UGBFInputComponent >( player_input_component );

            for ( const auto input_config_ptr : pawn_data->InputConfigs )
            {
                if ( const auto * input_config = input_config_ptr.LoadSynchronous() )
                {
                    input_component->AddInputMappings( input_config, enhanced_input_local_player_subsystem );

                    TArray< uint32 > bind_handles;
                    input_component->BindAbilityActions( input_config, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ bind_handles );

                    BindNativeActions( input_component, input_config );
                }
            }
            FModifyContextOptions options = {};
            options.bIgnoreAllPressedKeysUntilRelease = false;

            for ( const auto & mappable_config : pawn_data->MappableConfigs )
            {
                if ( const auto * config = mappable_config.Config.LoadSynchronous() )
                {
                    if ( mappable_config.bShouldActivateAutomatically && mappable_config.CanBeActivated() )
                    {
                        enhanced_input_local_player_subsystem->AddPlayerMappableConfig( config, options );
                    }
                }
            }
        }
    }

    if ( ensure( !bReadyToBindInputs ) )
    {
        bReadyToBindInputs = true;
    }

    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( const_cast< APlayerController * >( pc ), NAME_BindInputsNow );
    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( const_cast< APawn * >( pawn ), NAME_BindInputsNow );
}

void UGBFHeroComponent::Input_AbilityInputTagPressed( FGameplayTag input_tag )
{
    if ( const auto * pawn = GetPawn< APawn >() )
    {
        if ( const auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
        {
            if ( auto * asc = pawn_ext_comp->GetGASExtAbilitySystemComponent() )
            {
                asc->AbilityInputTagPressed( input_tag );
            }
        }
    }
}

void UGBFHeroComponent::Input_AbilityInputTagReleased( FGameplayTag input_tag )
{
    if ( const auto * pawn = GetPawn< APawn >() )
    {
        if ( const auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( pawn ) )
        {
            if ( auto * asc = pawn_ext_comp->GetGASExtAbilitySystemComponent() )
            {
                asc->AbilityInputTagReleased( input_tag );
            }
        }
    }
}

void UGBFHeroComponent::BindNativeActions( UGBFInputComponent * input_component, const UGBFInputConfig * input_config )
{
}