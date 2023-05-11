#include "Settings/CustomSettings/GBFSettingAction_SafeZoneEditor.h"

#include "DataSource/GameSettingDataSourceDynamic.h"
#include "Engine/GBFLocalPlayer.h"

#include <Settings/GBFGameSettingRegistry.h>
#include <Widgets/Layout/SSafeZone.h>

#define LOCTEXT_NAMESPACE "GBFSettings"

UGBFSettingAction_SafeZoneEditor::UGBFSettingAction_SafeZoneEditor()
{
    SafeZoneValueSetting = NewObject< UGBFSettingValueScalarDynamic_SafeZoneValue >();
    SafeZoneValueSetting->SetDevName( TEXT( "SafeZoneValue" ) );
    SafeZoneValueSetting->SetDisplayName( LOCTEXT( "SafeZoneValue_Name", "Safe Zone Value" ) );
    SafeZoneValueSetting->SetDescriptionRichText( LOCTEXT( "SafeZoneValue_Description", "The safezone area percentage." ) );
    SafeZoneValueSetting->SetDefaultValue( 0.0f );
    SafeZoneValueSetting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetSafeZone ) );
    SafeZoneValueSetting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetSafeZone ) );
    SafeZoneValueSetting->SetDisplayFormat( []( double SourceValue, double NormalizedValue ) {
        return FText::AsNumber( SourceValue );
    } );
    SafeZoneValueSetting->SetSettingParent( this );
}

TArray< UGameSetting * > UGBFSettingAction_SafeZoneEditor::GetChildSettings()
{
    return { SafeZoneValueSetting };
}

void UGBFSettingValueScalarDynamic_SafeZoneValue::ResetToDefault()
{
    Super::ResetToDefault();
    SSafeZone::SetGlobalSafeZoneScale( TOptional< float >( DefaultValue.Get( 0.0f ) ) );
}

void UGBFSettingValueScalarDynamic_SafeZoneValue::RestoreToInitial()
{
    Super::RestoreToInitial();
    SSafeZone::SetGlobalSafeZoneScale( TOptional< float >( InitialValue ) );
}

#undef LOCTEXT_NAMESPACE