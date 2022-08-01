#pragma once

#include <CoreMinimal.h>
#include <Editor/UnrealEdEngine.h>

#include "GBFEditorEngine.generated.h"

UCLASS()
class GAMEBASEFRAMEWORKEDITOR_API UGBFEditorEngine : public UUnrealEdEngine
{
    GENERATED_BODY()

public:
    void Tick( float delta_seconds, bool idle_mode ) override;

    FGameInstancePIEResult PreCreatePIEInstances( const bool any_blueprint_errors, const bool start_in_spectator_mode, const float pie_start_time, const bool supports_online_pie, int32 & num_online_pie_instances ) override;

protected:
    void StartPlayInEditorSession( FRequestPlaySessionParams & in_request_params ) override;

private:
    void FirstTickSetup();

    bool bFirstTickSetup = false;
};
