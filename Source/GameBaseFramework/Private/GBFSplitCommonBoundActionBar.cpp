#include "GBFSplitCommonBoundActionBar.h"

#include "CommonInputSubsystem.h"
#include "CommonInputTypeEnum.h"
#include "CommonUITypes.h"
#include "Editor/WidgetCompilerLog.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Input/CommonBoundActionButtonInterface.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Input/UIActionBinding.h"
#include "InputAction.h"
#include "OnlineSubsystemUtils.h"

bool bActionBarIgnoreOptOut = false;
static FAutoConsoleVariableRef CVarActionBarIgnoreOptOut(
    TEXT( "ActionBar.IgnoreOptOut" ),
    bActionBarIgnoreOptOut,
    TEXT( "If true, the Bound Action Bar will display bindings whether or not they are configured bDisplayInReflector" ),
    ECVF_Default );

void UGBFSplitCommonBoundActionBar::Tick( float delta_time )
{
    if ( bIsRefreshQueued )
    {
        HandleDeferredDisplayUpdate();
    }
}

ETickableTickType UGBFSplitCommonBoundActionBar::GetTickableTickType() const
{
    return ETickableTickType::Always;
}

TStatId UGBFSplitCommonBoundActionBar::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT( UGBFSplitCommonBoundActionBar, STATGROUP_Tickables )
}

bool UGBFSplitCommonBoundActionBar::IsTickableWhenPaused() const
{
    return true;
}
bool UGBFSplitCommonBoundActionBar::IsEntryClassValid( TSubclassOf< UUserWidget > in_entry_class ) const
{
    if ( in_entry_class )
    {
        // Would InEntryClass create an instance of the same DynamicEntryBox
        if ( UWidgetTree * widget_tree = Cast< UWidgetTree >( GetOuter() ) )
        {
            if ( UUserWidget * user_widget = Cast< UUserWidget >( widget_tree->GetOuter() ) )
            {
                if ( in_entry_class->IsChildOf( user_widget->GetClass() ) )
                {
                    return false;
                }
            }
        }
    }

    return true;
}

void UGBFSplitCommonBoundActionBar::OnWidgetRebuilt()
{
    Super::OnWidgetRebuilt();

    if ( const auto * game_instance = GetGameInstance() )
    {
        if ( game_instance->GetGameViewportClient() )
        {
            game_instance->GetGameViewportClient()->OnPlayerAdded().AddUObject( this, &UGBFSplitCommonBoundActionBar::HandlePlayerAdded );
        }

        for ( const auto * local_player : game_instance->GetLocalPlayers() )
        {
            MonitorPlayerActions( local_player );
        }

        // Establish entries (as needed) immediately upon construction
        HandleDeferredDisplayUpdate();
    }
}

void UGBFSplitCommonBoundActionBar::SynchronizeProperties()
{
    Super::SynchronizeProperties();
}

void UGBFSplitCommonBoundActionBar::ReleaseSlateResources( bool release_children )
{
    Super::ReleaseSlateResources( release_children );

    if ( const auto * game_instance = GetGameInstance() )
    {
        for ( const auto * local_player : game_instance->GetLocalPlayers() )
        {
            if ( const auto * action_router = ULocalPlayer::GetSubsystem< UCommonUIActionRouterBase >( local_player ) )
            {
                action_router->OnBoundActionsUpdated().RemoveAll( this );
            }
        }
    }
}

void UGBFSplitCommonBoundActionBar::AddEntryChild( UUserWidget & child_widget )
{
    if ( MyPanelWidget.IsValid() )
    {
        child_widget.TakeWidget();
    }
}

namespace UGBFSplitCommonBoundActionBarInternal
{
    TArray< TSubclassOf< UUserWidget >, TInlineAllocator< 4 > > recursive_detection;
}

UUserWidget * UGBFSplitCommonBoundActionBar::CreateEntryInternal( TSubclassOf< UUserWidget > in_entry_class )
{
    const auto has_recursive_user_widget = UGBFSplitCommonBoundActionBarInternal::recursive_detection.ContainsByPredicate( [ in_entry_class ]( TSubclassOf< UUserWidget > recursive_item ) {
        return in_entry_class->IsChildOf( recursive_item );
    } );
    if ( has_recursive_user_widget )
    {
        UE_LOG( LogSlate, Error, TEXT( "'%s' cannot be added to DynamicEntry '%s' because it is already a child and it would create a recurssion." ), *in_entry_class->GetName(), *UGBFSplitCommonBoundActionBarInternal::recursive_detection.Last()->GetName() );
#if 0
        for (TSubclassOf<UUserWidget> recursive_item : UGBFSplitCommonBoundActionBarInternal::recursive_detection)
        {
            UE_LOG(LogSlate, Log, TEXT("%s"), *recursive_item->GetName());
        }
#endif
        return nullptr;
    }
    UGBFSplitCommonBoundActionBarInternal::recursive_detection.Push( in_entry_class );

    UUserWidget * new_entry_widget = EntryWidgetPool.GetOrCreateInstance( in_entry_class );
    if ( MyPanelWidget.IsValid() )
    {
        // If we've already been constructed, immediately add the child to our panel widget
        AddEntryChild( *new_entry_widget );
    }

    UGBFSplitCommonBoundActionBarInternal::recursive_detection.Pop();
    return new_entry_widget;

    return in_entry_class.GetDefaultObject();
}

#if WITH_EDITOR
void UGBFSplitCommonBoundActionBar::ValidateCompiledDefaults( IWidgetCompilerLog & compile_log ) const
{
    Super::ValidateCompiledDefaults( compile_log );

    if ( !ActionButtonClass )
    {
        compile_log.Error( FText::FromString( FString::Printf( TEXT( "Error_BoundActionBar_MissingButtonClass, {0} has no ActionButtonClass specified." ) ) ) );
    }
    else if ( compile_log.GetContextClass() && ActionButtonClass->IsChildOf( compile_log.GetContextClass() ) )
    {
        compile_log.Error( FText::FromString( FString::Printf( TEXT( "Error_BoundActionBar_RecursiveButtonClass, {0} has a recursive ActionButtonClass specified (reference itself)." ) ) ) );
    }
}
#endif

void UGBFSplitCommonBoundActionBar::HandleBoundActionsUpdated( bool from_owning_player )
{
    if ( from_owning_player || !bDisplayOwningPlayerActionsOnly )
    {
        bIsRefreshQueued = true;
    }
}

void UGBFSplitCommonBoundActionBar::HandleDeferredDisplayUpdate()
{
    bIsRefreshQueued = false;
    //: TODO: implement a ResetFunction in this class
    // ResetInternal();

    const auto * game_instance = GetGameInstance();
    check( game_instance );
    const auto * owning_local_player = GetOwningLocalPlayer();

    auto sorted_players = game_instance->GetLocalPlayers();
    sorted_players.StableSort(
        [ &owning_local_player ]( const ULocalPlayer & PlayerA, const ULocalPlayer & PlayerB ) {
            return &PlayerA != owning_local_player;
        } );

    for ( const ULocalPlayer * LocalPlayer : sorted_players )
    {
        if ( LocalPlayer == owning_local_player || !bDisplayOwningPlayerActionsOnly )
        {
            if ( IsEntryClassValid( ActionButtonClass ) )
            {
                if ( const auto * action_router = ULocalPlayer::GetSubsystem< UCommonUIActionRouterBase >( owning_local_player ) )
                {
                    const auto & input_subsystem = action_router->GetInputSubsystem();
                    const auto player_input_type = input_subsystem.GetCurrentInputType();
                    const auto & player_gamepad_name = input_subsystem.GetCurrentGamepadName();

                    TSet< FName > accepted_bindings;
                    auto filtered_bindings = action_router->GatherActiveBindings().FilterByPredicate( [ action_router, player_input_type, player_gamepad_name, &accepted_bindings ]( const auto & handle ) mutable {
                        if ( auto binding = FUIActionBinding::FindBinding( handle ) )
                        {
                            if ( !binding->bDisplayInActionBar && !bActionBarIgnoreOptOut )
                            {
                                return false;
                            }

                            if ( CommonUI::IsEnhancedInputSupportEnabled() )
                            {
                                if ( auto input_action = binding->InputAction.Get() )
                                {
                                    return CommonUI::ActionValidForInputType( action_router->GetLocalPlayer(), player_input_type, input_action );
                                }
                            }

                            if ( auto * legacy_Data = binding->GetLegacyInputActionData() )
                            {
                                if ( !legacy_Data->CanDisplayInReflector( player_input_type, player_gamepad_name ) )
                                {
                                    return false;
                                }
                            }
                            else
                            {
                                return false;
                            }

                            bool already_accepted = false;
                            accepted_bindings.Add( binding->ActionName, &already_accepted );
                            return !already_accepted;
                        }

                        return false;
                    } );

                    Algo::Sort( filtered_bindings, [ action_router, player_input_type, player_gamepad_name ]( const FUIActionBindingHandle & handle_a, const FUIActionBindingHandle & handle_b ) {
                        auto binding_a = FUIActionBinding::FindBinding( handle_a );
                        auto binding_b = FUIActionBinding::FindBinding( handle_b );

                        if ( ensureMsgf( ( binding_a && binding_b ), TEXT( "The array filter above should enforce that there are no null bindings" ) ) )
                        {
                            auto is_key_back_action = [ action_router, player_input_type, player_gamepad_name ]( FCommonInputActionDataBase * legacy_data, const auto * input_action ) {
                                if ( legacy_data )
                                {
                                    auto key = legacy_data->GetInputTypeInfo( player_input_type, player_gamepad_name ).GetKey();

                                    if ( player_input_type == ECommonInputType::Touch )
                                    {
                                        if ( !key.IsValid() )
                                        {
                                            key = legacy_data->GetInputTypeInfo( ECommonInputType::MouseAndKeyboard, player_gamepad_name ).GetKey();
                                        }
                                    }

                                    return key == EKeys::Virtual_Back || key == EKeys::Escape || key == EKeys::Android_Back;
                                }
                                else if ( input_action )
                                {
                                    auto key = CommonUI::GetFirstKeyForInputType( action_router->GetLocalPlayer(), player_input_type, input_action );

                                    if ( player_input_type == ECommonInputType::Touch )
                                    {
                                        if ( !key.IsValid() )
                                        {
                                            key = CommonUI::GetFirstKeyForInputType( action_router->GetLocalPlayer(), ECommonInputType::MouseAndKeyboard, input_action );
                                        }
                                    }

                                    return key == EKeys::Virtual_Back || key == EKeys::Escape || key == EKeys::Android_Back;
                                }

                                return false;
                            };

                            auto get_navbar_priority = []( FCommonInputActionDataBase * legacy_data, const auto * input_action ) {
                                if ( legacy_data )
                                {
                                    return legacy_data->NavBarPriority;
                                }
                                else if ( input_action )
                                {
                                    if ( auto input_action_meta_data = CommonUI::GetEnhancedInputActionMetadata( input_action ) )
                                    {
                                        return input_action_meta_data->NavBarPriority;
                                    }
                                }

                                return 0;
                            };

                            auto legacy_data_a = binding_a->GetLegacyInputActionData();
                            auto legacy_data_b = binding_b->GetLegacyInputActionData();

                            const UInputAction * input_action_a = nullptr;
                            const UInputAction * input_action_b = nullptr;

                            if ( CommonUI::IsEnhancedInputSupportEnabled() )
                            {
                                input_action_a = binding_a->InputAction.Get();
                                input_action_b = binding_b->InputAction.Get();
                            }

                            bool is_valid_action_a = legacy_data_a || input_action_a;
                            bool is_valid_action_b = legacy_data_b || input_action_b;

                            if ( ensureMsgf( ( is_valid_action_a && is_valid_action_b ), TEXT( "Action bindings not displayed yet -- array filter enforces they are not included" ) ) )
                            {
                                bool a_is_back = is_key_back_action( legacy_data_a, input_action_a );
                                bool b_is_back = is_key_back_action( legacy_data_b, input_action_b );

                                if ( a_is_back && b_is_back )
                                {
                                    return false;
                                }

                                int32 nav_bar_priority_a = get_navbar_priority( legacy_data_a, input_action_a );
                                int32 nav_bar_priority_b = get_navbar_priority( legacy_data_b, input_action_b );

                                if ( nav_bar_priority_a != nav_bar_priority_b )
                                {
                                    return nav_bar_priority_a < nav_bar_priority_b;
                                }
                            }

                            return GetTypeHash( binding_a->Handle ) < GetTypeHash( binding_b->Handle );
                        }

                        return true;
                    } );

                    for ( int binding_index = 0; binding_index < filtered_bindings.Num() - 1; binding_index++ )
                    {
                        auto * action_button = Cast< ICommonBoundActionButtonInterface >( CreateEntryInternal( ActionButtonClass ) );

                        if ( ensure( action_button ) )
                        {
                            action_button->SetRepresentedAction( filtered_bindings[ binding_index ] );
                            NativeOnActionButtonCreated( action_button, filtered_bindings[ binding_index ] );
                        }
                    }

                    //: TODO: handle the last item of filtered_bindings wich is the back handler
                }
            }
        }
    }
}

void UGBFSplitCommonBoundActionBar::HandlePlayerAdded( int32 player_id )
{
    const ULocalPlayer * new_player = GetGameInstance()->GetLocalPlayerByIndex( player_id );
    MonitorPlayerActions( new_player );
    HandleBoundActionsUpdated( new_player == GetOwningLocalPlayer() );
}

void UGBFSplitCommonBoundActionBar::MonitorPlayerActions( const ULocalPlayer * new_player )
{
    if ( const auto * action_router = ULocalPlayer::GetSubsystem< UCommonUIActionRouterBase >( new_player ) )
    {
        action_router->OnBoundActionsUpdated().AddUObject( this, &UGBFSplitCommonBoundActionBar::HandleBoundActionsUpdated, new_player == GetOwningLocalPlayer() );
    }
}
