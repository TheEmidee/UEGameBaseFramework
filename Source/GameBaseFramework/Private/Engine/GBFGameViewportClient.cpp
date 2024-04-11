#include "Engine/GBFGameViewportClient.h"

#include <Components/Viewport.h>
#include <Engine/Engine.h>
#include <Engine/LocalPlayer.h>
#include <GameMapsSettings.h>
#include <InputKeyEventArgs.h>

FGBFViewPortPlayerOffset::FGBFViewPortPlayerOffset() :
    TopLeftX( 0 ),
    TopLeftY( 0 ),
    BottomRightX( 0 ),
    BottomRightY( 0 )
{
}

UGBFGameViewportClient::UGBFGameViewportClient()
{
}

void UGBFGameViewportClient::LayoutPlayers()
{
    Super::LayoutPlayers();

    if ( !SplitScreenLayoutOffset.IsValid() )
    {
        return;
    }

    FVector2D viewport_size;
    GetViewportSize( viewport_size );

    int top_left_x = viewport_size.X * SplitScreenLayoutOffset.TopLeftX;
    int top_left_y = viewport_size.Y * SplitScreenLayoutOffset.TopLeftY;
    int bottom_right_x = viewport_size.X * SplitScreenLayoutOffset.BottomRightX;
    int bottom_right_y = viewport_size.Y * SplitScreenLayoutOffset.BottomRightY;

    const FVector2D new_viewport_size(
        viewport_size.X - top_left_x - bottom_right_x,
        viewport_size.Y - top_left_y - bottom_right_y );

    const auto viewport_ratio = new_viewport_size / viewport_size;
    const FVector2D origin_offset(
        static_cast< float >( top_left_x ) / viewport_size.X,
        static_cast< float >( top_left_y ) / viewport_size.Y );

    const auto & player_list = GetOuterUEngine()->GetGamePlayers( this );
    const auto player_count = player_list.Num();
    const auto current_split = GetCurrentSplitscreenConfiguration();

    for ( auto * player : player_list )
    {
        player->Size *= viewport_ratio;

        const auto player_index = player->GetControllerId();

        switch ( current_split )
        {
            case ESplitScreenType::None:
            {
                player->Origin = origin_offset;
            }
            break;
            case ESplitScreenType::TwoPlayer_Horizontal:
            case ESplitScreenType::ThreePlayer_Horizontal:
            case ESplitScreenType::FourPlayer_Horizontal:
            {
                player->Origin.X = origin_offset.X;
                player->Origin.Y = viewport_ratio.Y / player_count * player_index + origin_offset.Y;
            }
            break;
            case ESplitScreenType::TwoPlayer_Vertical:
            case ESplitScreenType::ThreePlayer_Vertical:
            case ESplitScreenType::FourPlayer_Vertical:
            {
                player->Origin.X = viewport_ratio.X / player_count * player_index + origin_offset.X;
                player->Origin.Y = origin_offset.Y;
            }
            break;
            case ESplitScreenType::ThreePlayer_FavorTop:
            {
                if ( player_index == 0 )
                {
                    player->Origin = origin_offset;
                }
                else
                {
                    player->Origin.X = viewport_ratio.X / 2 * ( player_index - 1 ) + origin_offset.X;
                    player->Origin.Y = viewport_ratio.Y / 2 + origin_offset.Y;
                }
            }
            break;
            case ESplitScreenType::ThreePlayer_FavorBottom:
            {
                if ( player_index == 2 )
                {
                    player->Origin.X = origin_offset.X;
                    player->Origin.Y = viewport_ratio.Y / 2 + origin_offset.Y;
                }
                else
                {
                    player->Origin.X = viewport_ratio.X / 2 * player_index + origin_offset.X;
                    player->Origin.Y = origin_offset.Y;
                }
            }
            break;
            case ESplitScreenType::FourPlayer_Grid:
            {
                if ( player_index < 2 )
                {
                    player->Origin.X = viewport_ratio.X / 2 * player_index + origin_offset.X;
                    player->Origin.Y = origin_offset.Y;
                }
                else
                {
                    player->Origin.X = viewport_ratio.X / 2 * ( player_index - 2 ) + origin_offset.X;
                    player->Origin.Y = viewport_ratio.Y / 2 + origin_offset.Y;
                }
            }
            break;
            case ESplitScreenType::SplitTypeCount:
            {
                checkNoEntry()
            }
            break;
            default:
            {
            }
        }
    }
}

void UGBFGameViewportClient::RemapControllerInput( FInputKeyEventArgs & key_event )
{
    if ( const auto num_local_players = World ? World->GetGameInstance()->GetNumLocalPlayers() : 0;
         num_local_players > 1 && key_event.Key.IsGamepadKey() && GetDefault< UGameMapsSettings >()->bOffsetPlayerGamepadIds )
    {
        auto id = key_event.InputDevice.GetId();
        id++;
        key_event.InputDevice = FInputDeviceId::CreateFromInternalId( id );

        if ( auto & device_mapper = IPlatformInputDeviceMapper::Get();
             !device_mapper.GetUserForInputDevice( key_event.InputDevice ).IsValid() )
        {
            device_mapper.Internal_MapInputDeviceToUser( key_event.InputDevice, FPlatformUserId::CreateFromInternalId( 1 ), EInputDeviceConnectionState::Connected );
        }
    }
    else if ( key_event.Viewport->IsPlayInEditorViewport() && key_event.Key.IsGamepadKey() )
    {
        GEngine->RemapGamepadControllerIdForPIE( this, key_event.ControllerId );
    }
}
