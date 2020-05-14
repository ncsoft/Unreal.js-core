#pragma once

#include "JavascriptEdModeLibrary.h"
#include "JavascriptEditorLibrary.h"
#include "JavascriptEdMode.generated.h"

/** Outcomes when determining whether it's possible to perform an action on the edit modes*/
UENUM()
enum class EJavascriptEditAction : uint8
{
	/** Can't process this action */
	Skip = 0,
	/** Can process this action */
	Process,
	/** Stop evaluating other modes (early out) */
	Halt,
};


UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEdMode : public UObject
{
	GENERATED_BODY()

public:	
#if WITH_EDITOR
	DECLARE_DYNAMIC_DELEGATE_RetVal(UWidget*, FOnGetWidget);
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSelectionChanged, FJavascriptEditorModeTools&, Tools, UObject*, Item);

	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FOnProcess, FName, Request, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(EJavascriptEditAction, FOnGetAction, FName, Request, FJavascriptEditorMode, Instance);
	
	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FOnUsesToolkits);
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnIsCompatibleWith, FName, bIsCompatibleWith);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FSimpleDelegate, FJavascriptEditorMode, Instance);	
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FVector,FQueryVector, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_FourParams(FActorDuplicated, TArray<AActor*>&, PreDuplicateSelection, TArray<AActor*>&, PostDuplicateSelection, bool, bOffsetLocations, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams(bool, FViewportXY, const FJavascriptEdViewport&, Viewport, int32, X, int32, Y, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FViewport0, const FJavascriptEdViewport&, Viewport, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams(bool, FViewportKey, const FJavascriptEdViewport&, Viewport, FKey, Key, EInputEvent, Event, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_SixParams(bool, FViewportAxis, const FJavascriptEdViewport&, Viewport, int32, ControllerId, FKey, Key, float, Delta, float, DeltaTime, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_FiveParams(bool, FViewportDelta, const FJavascriptEdViewport&, Viewport, FVector&, Drag, FRotator&, Rot, FVector&, Scale, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_ThreeParams(bool, FOnClick, const FJavascriptViewportClick&, ViewportClick, const FJavascriptHitProxy&, HitProxy, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_ThreeParams(bool, FOnSelect, AActor*, Actor, bool, bSelected, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnDraw, const FJavascriptPDI&, PDI, FJavascriptEditorMode, Instance);
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnDrawHUD, UCanvas*, Canvas, FJavascriptEditorMode, Instance);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FQueryVector OnGetWidgetLocation;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnSelect OnSelect;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnDraw OnDraw;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnDrawHUD OnDrawHUD;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnSelect IsSelectionAllowed;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnClick OnClick;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnProcess OnQuery;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewport0 OnStartTracking;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewport0 OnEndTracking;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewportAxis OnInputAxis;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewportKey OnInputKey;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewportDelta OnInputDelta;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewportXY OnCapturedMouseMove;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewportXY OnMouseEnter;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewport0 OnMouseLeave;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewportXY OnMouseMove;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewport0 OnLostFocus;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FViewport0 OnReceivedFocus;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnSelectionChanged OnSelectionChanged;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetWidget OnGetContent;	

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnProcess OnProcess;	

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetAction OnGetAction;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnUsesToolkits OnUsesToolkits;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnIsCompatibleWith OnIsCompatibleWith;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FSimpleDelegate OnActorMoved;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FActorDuplicated OnActorsDuplicated;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FSimpleDelegate OnActorSelectionChanged;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FSimpleDelegate OnActorPropChanged;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FSimpleDelegate OnMapChanged;

	UPROPERTY()
	FName ModeId;

	UPROPERTY()
	FJavascriptSlateIcon SlateIcon;

	UPROPERTY()
	FText ModeName;

	UPROPERTY()
	bool bVisible;

	UPROPERTY()
	int32 PriorityOrder;

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Register();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Unregister();
#endif
};
