#pragma once

#include "CommonGameViewportClient.h"
#include "Components/Viewport.h"

#include <CoreMinimal.h>

#include "GBFGameViewportClient.generated.h"

USTRUCT( BlueprintType )
struct FGBFViewPortPlayerOffset
{
    GENERATED_BODY()

    FGBFViewPortPlayerOffset();

    bool IsValid() const;

    UPROPERTY( BlueprintReadWrite )
    int TopLeftX;

    UPROPERTY( BlueprintReadWrite )
    int TopLeftY;

    UPROPERTY( BlueprintReadWrite )
    int BottomRightX;

    UPROPERTY( BlueprintReadWrite )
    int BottomRightY;
};

FORCEINLINE bool FGBFViewPortPlayerOffset::IsValid() const
{
    return TopLeftX != 0 || TopLeftY != 0 || BottomRightX != 0 || BottomRightY != 0;
}

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameViewportClient : public UCommonGameViewportClient
{
    GENERATED_BODY()

public:
    UGBFGameViewportClient();

    void SetSplitScreenLayoutOffset( FGBFViewPortPlayerOffset offset );
    void LayoutPlayers() override;

private:
    FGBFViewPortPlayerOffset SplitScreenLayoutOffset;
};
