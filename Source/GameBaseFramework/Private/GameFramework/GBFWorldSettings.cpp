#include "GameFramework/GBFWorldSettings.h"

#include "Engine/AssetManager.h"

AGBFWorldSettings::AGBFWorldSettings()
{
#if WITH_EDITORONLY_DATA
    bForceStandaloneNetMode = false;
#endif
}