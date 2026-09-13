#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/HronoTutorialTypes.h"
#include "HronoTutorialWidget.generated.h"

class UBackgroundBlur;
class UBorder;
class UButton;
class UImage;
class UScrollBox;
class UTextBlock;
class UTexture2D;
class IInputProcessor;

/**
 * Local-only equipment journal and first-pickup prompt. The complete default
 * layout is created in C++, so no Widget Blueprint is required.
 */
UCLASS()
class HRONO_API UHronoTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UHronoTutorialWidget(const FObjectInitializer& ObjectInitializer);

	void ShowPickupPrompt(EHronoTutorialItem Item);
	void SetItemDiscovered(EHronoTutorialItem Item, bool bDiscovered = true);
	void SetGameStageText(const FText& NewStageText);
	void ToggleMenu();
	void OpenMenu(EHronoTutorialItem Item = EHronoTutorialItem::None);
	void CloseMenu();
	/** Raw Slate fallback used while the modal is open; independent of UMG focus/hit routing. */
	bool HandleMenuMouseButtonDown(const FPointerEvent& MouseEvent);
	/** Routes wheel input directly to the tutorial scroll area under the cursor. */
	bool HandleMenuMouseWheel(const FPointerEvent& MouseEvent);

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool IsMenuOpen() const { return bMenuOpen; }

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void BuildWidgetTree();
	void BindButtons();
	void SelectTutorial(EHronoTutorialItem Item);
	void RefreshNavigation();
	void HidePickupPrompt();
	UTexture2D* LoadTutorialTexture(EHronoTutorialItem Item);
	void RegisterMenuInputPreProcessor();
	void UnregisterMenuInputPreProcessor();

	UFUNCTION()
	void HandleMonocleClicked();
	UFUNCTION()
	void HandleDosimeterClicked();
	UFUNCTION()
	void HandleClockClicked();
	UFUNCTION()
	void HandleSkullClicked();
	UFUNCTION()
	void HandleTableRitualClicked();
	UFUNCTION()
	void HandleMirrorClicked();
	UFUNCTION()
	void HandleAxeClicked();
	UFUNCTION()
	void HandleCloseClicked();

	UPROPERTY(Transient)
	TObjectPtr<UBackgroundBlur> MenuBlur;
	UPROPERTY(Transient)
	TObjectPtr<UBorder> MenuLayer;
	UPROPERTY(Transient)
	TObjectPtr<UBorder> PickupPrompt;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PickupPromptText;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StageText;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CollectionText;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ItemTitleText;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ItemSubtitleText;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ItemDescriptionText;
	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> DescriptionScroll;
	UPROPERTY(Transient)
	TObjectPtr<UScrollBox> NavigationScroll;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ItemControlText;
	UPROPERTY(Transient)
	TObjectPtr<UImage> TutorialImage;
	UPROPERTY(Transient)
	TObjectPtr<UImage> SecondaryTutorialImage;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MissingImageText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> MonocleButton;
	UPROPERTY(Transient)
	TObjectPtr<UButton> DosimeterButton;
	UPROPERTY(Transient)
	TObjectPtr<UButton> ClockButton;
	UPROPERTY(Transient)
	TObjectPtr<UButton> SkullButton;
	UPROPERTY(Transient)
	TObjectPtr<UButton> TableRitualButton;
	UPROPERTY(Transient)
	TObjectPtr<UButton> MirrorButton;
	UPROPERTY(Transient)
	TObjectPtr<UButton> AxeButton;
	UPROPERTY(Transient)
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(Transient)
	TMap<EHronoTutorialItem, TObjectPtr<UTexture2D>> LoadedTextures;
	/** Correct shared anomaly example. Hard-referenced so it is included in packaged builds. */
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Monocle")
	TObjectPtr<UTexture2D> MonocleCorrectTexture;
	/** Incorrect one-sided anomaly example. Hard-referenced so it is included in packaged builds. */
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Monocle")
	TObjectPtr<UTexture2D> MonocleWrongTexture;
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Dosimeter")
	TObjectPtr<UTexture2D> DosimeterCorrectTexture;
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Dosimeter")
	TObjectPtr<UTexture2D> DosimeterWrongTexture;
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Clock")
	TObjectPtr<UTexture2D> ClockCorrectTexture;
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Clock")
	TObjectPtr<UTexture2D> ClockWrongTexture;
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Skull")
	TObjectPtr<UTexture2D> SkullTutorialTexture;
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|TableRitual")
	TObjectPtr<UTexture2D> TableRitualTexture;
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial|Mirror")
	TObjectPtr<UTexture2D> MirrorTutorialTexture;

	TSet<EHronoTutorialItem> DiscoveredItems;
	TSharedPtr<IInputProcessor> MenuInputPreProcessor;
	EHronoTutorialItem SelectedItem = EHronoTutorialItem::Monocle;
	EHronoTutorialItem PendingPromptItem = EHronoTutorialItem::None;
	FTimerHandle PromptTimer;
	bool bMenuOpen = false;
	bool bOwnsGamePause = false;
	bool bPreviousMouseCursor = false;
	bool bPreviousClickEvents = false;
	bool bPreviousMouseOverEvents = false;
};
