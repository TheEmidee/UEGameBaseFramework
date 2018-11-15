#pragma once

#include "IDetailCustomization.h"

// Adds a button in the GameBaseFramework settings panel, to reload input the textures
class FGameBaseFrameworkSettingsDetails : public IDetailCustomization
{
public:

    void CustomizeDetails( IDetailLayoutBuilder & detail_builder ) override;

    static TSharedRef< IDetailCustomization > MakeInstance();
};
