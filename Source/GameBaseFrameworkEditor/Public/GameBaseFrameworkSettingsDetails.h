#pragma once

#include "IDetailCustomization.h"

class FGameBaseFrameworkSettingsDetails : public IDetailCustomization
{
public:    
    
    virtual void CustomizeDetails( IDetailLayoutBuilder & detail_builder ) override;

    static TSharedRef<IDetailCustomization> MakeInstance();
};