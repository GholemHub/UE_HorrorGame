#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HronoFpsWidget.generated.h"

class UTextBlock;

/** Lightweight local-only FPS overlay that also works in Shipping builds. */
UCLASS()
class HRONO_API UHronoFpsWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	UHronoFpsWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildWidgetTree();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FpsText;

	float AccumulatedTime = 0.0f;
	int32 AccumulatedFrames = 0;
};
