#include "GameBaseFrameworkSettingsDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"

#include "IGameBaseFrameworkModule.h"
#include "GameBaseFrameworkSettings.h"


#define LOCTEXT_NAMESPACE "GameBaseFrameworkSettingsDetails"

TSharedRef<IDetailCustomization> FGameBaseFrameworkSettingsDetails::MakeInstance()
{
    return MakeShareable( new FGameBaseFrameworkSettingsDetails );
}

void FGameBaseFrameworkSettingsDetails::CustomizeDetails( IDetailLayoutBuilder& detail_builder )
{
    TArray<TWeakObjectPtr<UObject>> objects_being_customized;
    detail_builder.GetObjectsBeingCustomized(objects_being_customized);
    check(objects_being_customized.Num() == 1);

    TWeakObjectPtr<UGameBaseFrameworkSettings> settings = Cast< UGameBaseFrameworkSettings >( objects_being_customized[ 0 ].Get() );

    IDetailCategoryBuilder & input_textures_category = detail_builder.EditCategory( "InputTextures" );

    input_textures_category.AddCustomRow( LOCTEXT( "PlatformTextures", "PlatformTextures" ) )
        .ValueContent()
        [
            SNew( SHorizontalBox )
            + SHorizontalBox::Slot()
            .Padding(5)
            .AutoWidth()
            [
                SNew( SButton )
                .Text( LOCTEXT( "ReloadPlatformInputTextures", "Reload platform input textures" ) )
                .ToolTipText( LOCTEXT( "ReloadPlatformInputTextures_Tooltip", "Reload the platform input textures (useful when you change the content of the CSV files)" ) )
                .OnClicked_Lambda( [this, settings ]()
                {
                    IGameBaseFrameworkModule::Get().RefreshPlatformInputTextures();
                    return FReply::Handled();
                })
            ]
        ];
}

#undef LOCTEXT_NAMESPACE