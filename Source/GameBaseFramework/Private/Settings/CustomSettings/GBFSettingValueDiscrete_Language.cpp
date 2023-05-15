#include "Settings/CustomSettings/GBFSettingValueDiscrete_Language.h"

#include "Engine/GBFLocalPlayer.h"
#include "Messaging/CommonGameDialog.h"
#include "Messaging/CommonMessagingSubsystem.h"

#include <Internationalization/Culture.h>

#define LOCTEXT_NAMESPACE "GBF"

static const int32 SettingSystemDefaultLanguageIndex = 0;

void UGBFSettingValueDiscrete_Language::StoreInitial()
{
}

void UGBFSettingValueDiscrete_Language::ResetToDefault()
{
    SetDiscreteOptionByIndex( SettingSystemDefaultLanguageIndex );
}

void UGBFSettingValueDiscrete_Language::RestoreToInitial()
{
    if ( auto * settings = CastChecked< UGBFLocalPlayer >( LocalPlayer )->GetSharedSettings() )
    {
        settings->ClearPendingCulture();
        NotifySettingChanged( EGameSettingChangeReason::RestoreToInitial );
    }
}

void UGBFSettingValueDiscrete_Language::SetDiscreteOptionByIndex( const int32 index )
{
    if ( auto * settings = CastChecked< UGBFLocalPlayer >( LocalPlayer )->GetSharedSettings() )
    {
        if ( index == SettingSystemDefaultLanguageIndex )
        {
            settings->ResetToDefaultCulture();
        }
        else if ( AvailableCultureNames.IsValidIndex( index ) )
        {
            settings->SetPendingCulture( AvailableCultureNames[ index ] );
        }

        NotifySettingChanged( EGameSettingChangeReason::Change );
    }
}

int32 UGBFSettingValueDiscrete_Language::GetDiscreteOptionIndex() const
{
    if ( const auto * settings = CastChecked< UGBFLocalPlayer >( LocalPlayer )->GetSharedSettings() )
    {
        if ( settings->ShouldResetToDefaultCulture() )
        {
            return SettingSystemDefaultLanguageIndex;
        }

        // We prefer the pending culture to the current culture as the options UI updates the pending culture before it
        // gets applied, and we need the UI to reflect that choice
        FString pending_culture = settings->GetPendingCulture();
        if ( pending_culture.IsEmpty() )
        {
            if ( settings->IsUsingDefaultCulture() )
            {
                return SettingSystemDefaultLanguageIndex;
            }

            pending_culture = FInternationalization::Get().GetCurrentCulture()->GetName();
        }

        // Try to find an exact match
        {
            if ( const int32 exact_match_index = AvailableCultureNames.IndexOfByKey( pending_culture );
                 exact_match_index != INDEX_NONE )
            {
                return exact_match_index;
            }
        }

        // Try to find a prioritized match (eg, allowing "en-US" to show as "en" in the UI)
        const auto prioritized_pending_cultures = FInternationalization::Get().GetPrioritizedCultureNames( pending_culture );
        for ( auto i = 0; i < AvailableCultureNames.Num(); ++i )
        {
            if ( prioritized_pending_cultures.Contains( AvailableCultureNames[ i ] ) )
            {
                return i;
            }
        }
    }

    return 0;
}

TArray< FText > UGBFSettingValueDiscrete_Language::GetDiscreteOptions() const
{
    TArray< FText > options;

    for ( const auto & culture_name : AvailableCultureNames )
    {
        if ( culture_name == TEXT( "" ) )
        {
            const auto system_default_culture = FInternationalization::Get().GetDefaultCulture();
            const auto & default_culture_display_name = system_default_culture->GetDisplayName();
            auto localized_system_default = FText::Format( LOCTEXT( "SystemDefaultLanguage", "System Default ({0})" ), FText::FromString( default_culture_display_name ) );

            options.Add( MoveTemp( localized_system_default ) );
        }
        else
        {
            if ( const auto culture = FInternationalization::Get().GetCulture( culture_name );
                 ensureMsgf( culture != nullptr, TEXT( "Unable to find Culture '%s'!" ), *culture_name ) )
            {
                const auto culture_display_name = culture->GetDisplayName();
                const auto culture_native_name = culture->GetNativeName();

                // Only show both names if they're different (to avoid repetition)
                auto entry = ( !culture_native_name.Equals( culture_display_name, ESearchCase::CaseSensitive ) )
                                 ? FString::Printf( TEXT( "%s (%s)" ), *culture_native_name, *culture_display_name )
                                 : culture_native_name;

                options.Add( FText::FromString( entry ) );
            }
        }
    }

    return options;
}

void UGBFSettingValueDiscrete_Language::OnInitialized()
{
    Super::OnInitialized();

    const auto all_culture_names = FTextLocalizationManager::Get().GetLocalizedCultureNames( ELocalizationLoadFlags::Game );
    for ( const auto & culture_name : all_culture_names )
    {
        if ( FInternationalization::Get().IsCultureAllowed( culture_name ) )
        {
            AvailableCultureNames.Add( culture_name );
        }
    }

    AvailableCultureNames.Insert( TEXT( "" ), SettingSystemDefaultLanguageIndex );
}

void UGBFSettingValueDiscrete_Language::OnApply()
{
    if ( auto * messaging = LocalPlayer->GetSubsystem< UCommonMessagingSubsystem >() )
    {
        messaging->ShowConfirmation(
            UCommonGameDialogDescriptor::CreateConfirmationOk(
                LOCTEXT( "WarningLanguage_Title", "Language Changed" ),
                LOCTEXT( "WarningLanguage_Message", "You will need to restart the game completely for all language related changes to take effect." ) ) );
    }
}

#undef LOCTEXT_NAMESPACE