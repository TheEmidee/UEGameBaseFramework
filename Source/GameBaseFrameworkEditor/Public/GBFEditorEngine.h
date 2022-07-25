#pragma once

#include <CoreMinimal.h>
#include <Editor/UnrealEdEngine.h>

#include "GBFEditorEngine.generated.h"

UCLASS()
class GAMEBASEFRAMEWORKEDITOR_API UGBFEditorEngine : public UUnrealEdEngine
{
    GENERATED_BODY()

public:
    void Init( IEngineLoop * in_engine_loop ) override;
    void Start() override;
    void Tick( float delta_seconds, bool idle_mode ) override;

    // :TODO: UE5
    // FGameInstancePIEResult PreCreatePIEInstances( const bool bAnyBlueprintErrors, const bool bStartInSpectatorMode, const float PIEStartTime, const bool bSupportsOnlinePIE, int32 & InNumOnlinePIEInstances ) override;

protected:
    void StartPlayInEditorSession( FRequestPlaySessionParams & in_request_params ) override;

private:
    void FirstTickSetup();

    bool bFirstTickSetup = false;
};
