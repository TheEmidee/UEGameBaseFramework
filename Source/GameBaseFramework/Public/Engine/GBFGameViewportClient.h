#pragma once

#include "CommonGameViewportClient.h"

#include <CoreMinimal.h>

#include "GBFGameViewportClient.generated.h"

USTRUCT( BlueprintType )
struct FGBFViewPortPlayerOffset
{
    GENERATED_BODY()

    FGBFViewPortPlayerOffset();

    FGBFViewPortPlayerOffset( int top_left_x, int top_left_y, int bottom_right_x, int bottom_right_y ) :
        TopLeftX( top_left_x ),
        TopLeftY( top_left_y ),
        BottomRightX( bottom_right_x ),
        BottomRightY( bottom_right_y )
    {
    }

    bool IsValid() const;

    /**
     * Linearly interpolates between two viewport offsets.
     *
     * @param A     The starting offset.
     * @param B     The ending offset.
     * @param Alpha The interpolation factor. Clamped between 0 and 1.
     * @return The interpolated viewport offset.
     */
    static FGBFViewPortPlayerOffset Lerp( const FGBFViewPortPlayerOffset & A, const FGBFViewPortPlayerOffset & B, float Alpha );

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

FORCEINLINE FGBFViewPortPlayerOffset FGBFViewPortPlayerOffset::Lerp( const FGBFViewPortPlayerOffset & A, const FGBFViewPortPlayerOffset & B, float Alpha )
{
    return FGBFViewPortPlayerOffset {
        FMath::Lerp( A.TopLeftX, B.TopLeftX, Alpha ),
        FMath::Lerp( A.TopLeftY, B.TopLeftY, Alpha ),
        FMath::Lerp( A.BottomRightX, B.BottomRightX, Alpha ),
        FMath::Lerp( A.BottomRightY, B.BottomRightY, Alpha )
    };
}

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameViewportClient : public UCommonGameViewportClient
{
    GENERATED_BODY()

public:
    UGBFGameViewportClient();

    void SetSplitScreenLayoutOffset( const FGBFViewPortPlayerOffset & offset );
    const FGBFViewPortPlayerOffset & GetSplitScreenLayoutOffset() const;
    void LayoutPlayers() override;


private:
    FGBFViewPortPlayerOffset SplitScreenLayoutOffset;
};

FORCEINLINE void UGBFGameViewportClient::SetSplitScreenLayoutOffset( const FGBFViewPortPlayerOffset & offset )
{
    SplitScreenLayoutOffset = offset;
}

FORCEINLINE const FGBFViewPortPlayerOffset & UGBFGameViewportClient::GetSplitScreenLayoutOffset() const
{
    return SplitScreenLayoutOffset;
}