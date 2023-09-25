#include "Settings/CustomSettings/GBFSettingValueKeyboardInput.h"

#include "Engine/GBFLocalPlayer.h"
#include "Settings/GBFGameUserSettings.h"

#include <EnhancedInputSubsystems.h>

#define LOCTEXT_NAMESPACE "GBFSettings"

namespace GBF::ErrorMessages
{
    static const FText UnknownMappingName = LOCTEXT( "GBFErrors_UnknownMappingName", "Unknown Mapping" );
}

UGBFSettingValueKeyboardInput::UGBFSettingValueKeyboardInput()
{
    bReportAnalytics = false;
}

FText UGBFSettingValueKeyboardInput::GetSettingDisplayName() const
{
    if ( const auto * row = FindKeyMappingRow() )
    {
        if ( row->HasAnyMappings() )
        {
            return row->Mappings.begin()->GetDisplayName();
        }
    }

    return GBF::ErrorMessages::UnknownMappingName;
}

FText UGBFSettingValueKeyboardInput::GetSettingDisplayCategory() const
{
    if ( const auto * row = FindKeyMappingRow() )
    {
        if ( row->HasAnyMappings() )
        {
            return row->Mappings.begin()->GetDisplayCategory();
        }
    }

    return GBF::ErrorMessages::UnknownMappingName;
}

const FKeyMappingRow * UGBFSettingValueKeyboardInput::FindKeyMappingRow() const
{
    if ( const auto * profile = FindMappableKeyProfile() )
    {
        return profile->FindKeyMappingRow( ActionMappingName );
    }

    ensure( false );
    return nullptr;
}

UEnhancedPlayerMappableKeyProfile * UGBFSettingValueKeyboardInput::FindMappableKeyProfile() const
{
    if ( const auto * settings = GetUserSettings() )
    {
        return settings->GetKeyProfileWithIdentifier( ProfileIdentifier );
    }

    ensure( false );
    return nullptr;
}

UEnhancedInputUserSettings * UGBFSettingValueKeyboardInput::GetUserSettings() const
{
    if ( const auto * local_player = Cast< UGBFLocalPlayer >( LocalPlayer ) )
    {
        // Map the key to the player key profile
        if ( const auto * system = local_player->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
        {
            return system->GetUserSettings();
        }
    }

    return nullptr;
}

void UGBFSettingValueKeyboardInput::OnInitialized()
{
    DynamicDetails = FGetGameSettingsDetails::CreateLambda( [ this ]( ULocalPlayer & ) {
        if ( const auto * row = FindKeyMappingRow() )
        {
            if ( row->HasAnyMappings() )
            {
                return FText::Format( LOCTEXT( "DynamicDetails_KeyboardInputAction", "Bindings for {0}" ), row->Mappings.begin()->GetDisplayName() );
            }
        }
        return FText::GetEmpty();
    } );

    Super::OnInitialized();
}

void UGBFSettingValueKeyboardInput::InitializeInputData( const UEnhancedPlayerMappableKeyProfile * key_profile, const FKeyMappingRow & mapping_data, const FPlayerMappableKeyQueryOptions & InQueryOptions )
{
    check( key_profile );

    ProfileIdentifier = key_profile->GetProfileIdentifer();
    QueryOptions = InQueryOptions;

    for ( const auto & mapping : mapping_data.Mappings )
    {
        // Only add mappings that pass the query filters that have been provided upon creation
        if ( !key_profile->DoesMappingPassQueryOptions( mapping, QueryOptions ) )
        {
            continue;
        }

        ActionMappingName = mapping.GetMappingName();
        InitialKeyMappings.Add( mapping.GetSlot(), mapping.GetCurrentKey() );
        const auto & mapping_display_name = mapping.GetDisplayName();

        if ( !mapping_display_name.IsEmpty() )
        {
            SetDisplayName( mapping_display_name );
        }
    }

    const auto name_string = TEXT( "KBM_Input_" ) + ActionMappingName.ToString();
    SetDevName( *name_string );
}

FText UGBFSettingValueKeyboardInput::GetKeyTextFromSlot( const EPlayerMappableKeySlot slot ) const
{
    if ( const auto * profile = FindMappableKeyProfile() )
    {
        auto query_options_for_slot = QueryOptions;
        query_options_for_slot.SlotToMatch = slot;

        if ( const auto * row = FindKeyMappingRow() )
        {
            for ( const auto & mapping : row->Mappings )
            {
                if ( profile->DoesMappingPassQueryOptions( mapping, query_options_for_slot ) )
                {
                    return mapping.GetCurrentKey().GetDisplayName();
                }
            }
        }
    }

    return EKeys::Invalid.GetDisplayName();
}

PRAGMA_DISABLE_DEPRECATION_WARNINGS
FText UGBFSettingValueKeyboardInput::GetPrimaryKeyText() const
{
    return GetKeyTextFromSlot( EPlayerMappableKeySlot::First );
}

FText UGBFSettingValueKeyboardInput::GetSecondaryKeyText() const
{
    return GetKeyTextFromSlot( EPlayerMappableKeySlot::Second );
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS

void UGBFSettingValueKeyboardInput::ResetToDefault()
{
    if ( auto * settings = GetUserSettings() )
    {
        FMapPlayerKeyArgs args = {};
        args.MappingName = ActionMappingName;

        FGameplayTagContainer failure_reason;
        settings->ResetAllPlayerKeysInRow( args, failure_reason );

        NotifySettingChanged( EGameSettingChangeReason::Change );
    }
}

void UGBFSettingValueKeyboardInput::StoreInitial()
{
    if ( const auto * profile = FindMappableKeyProfile() )
    {
        if ( const auto * row = FindKeyMappingRow() )
        {
            for ( const auto & mapping : row->Mappings )
            {
                if ( profile->DoesMappingPassQueryOptions( mapping, QueryOptions ) )
                {
                    ActionMappingName = mapping.GetMappingName();
                    InitialKeyMappings.Add( mapping.GetSlot(), mapping.GetCurrentKey() );
                }
            }
        }
    }
}

void UGBFSettingValueKeyboardInput::RestoreToInitial()
{
    for ( auto & [ key_slot, key ] : InitialKeyMappings )
    {
        ChangeBinding( static_cast< int32 >( key_slot ), key );
    }
}

bool UGBFSettingValueKeyboardInput::ChangeBinding( const int32 key_bind_slot, const FKey & new_key )
{
    if ( !new_key.IsGamepadKey() )
    {
        FMapPlayerKeyArgs args = {};
        args.MappingName = ActionMappingName;
        args.Slot = static_cast< EPlayerMappableKeySlot >( static_cast< uint8 >( key_bind_slot ) );
        args.NewKey = new_key;
        // If you want to, you can additionally specify this mapping to only be applied to a certain hardware device or key profile
        // Args.ProfileId =
        // Args.HardwareDeviceId =

        if ( auto * settings = GetUserSettings() )
        {
            FGameplayTagContainer failure_reason;
            settings->MapPlayerKey( args, failure_reason );
            NotifySettingChanged( EGameSettingChangeReason::Change );
        }

        return true;
    }

    return false;
}

void UGBFSettingValueKeyboardInput::GetAllMappedActionsFromKey( int32 /*key_bind_slot*/, const FKey & key, TArray< FName > & out_action_names ) const
{
    if ( const auto * profile = FindMappableKeyProfile() )
    {
        profile->GetMappingNamesForKey( key, out_action_names );
    }
}

bool UGBFSettingValueKeyboardInput::IsMappingCustomized() const
{
    bool result = false;

    if ( const auto * profile = FindMappableKeyProfile() )
    {
        const auto query_options_for_slot = QueryOptions;

        if ( const auto * row = FindKeyMappingRow() )
        {
            for ( const auto & mapping : row->Mappings )
            {
                if ( profile->DoesMappingPassQueryOptions( mapping, query_options_for_slot ) )
                {
                    result |= mapping.IsCustomized();
                }
            }
        }
    }

    return result;
}

#undef LOCTEXT_NAMESPACE