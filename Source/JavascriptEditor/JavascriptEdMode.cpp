PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#include "JavascriptEdMode.h"

#if WITH_EDITOR
#include "Components/Widget.h"
#include "EditorModeManager.h"
#include "EdMode.h"
#include "Engine/Canvas.h"
#include "JavascriptUMG/SJavascriptBox.h"
#include "Toolkits/ToolkitManager.h"

/** Util to find named canvas in transient package, and create if not found */
static UCanvas* GetCanvasByName(FName CanvasName)
{
    // Cache to avoid FString/FName conversions/compares
    static TMap<FName, UCanvas*> CanvasMap;
    UCanvas** FoundCanvas = CanvasMap.Find(CanvasName);
    if (!FoundCanvas)
    {
        UCanvas* CanvasObject = FindObject<UCanvas>(GetTransientPackage(), *CanvasName.ToString());
        if (!CanvasObject)
        {
            CanvasObject = NewObject<UCanvas>(GetTransientPackage(), CanvasName);
            CanvasObject->AddToRoot();
        }

        CanvasMap.Add(CanvasName, CanvasObject);
        return CanvasObject;
    }

    return *FoundCanvas;
}

class FJavascriptEdToolkit : public FModeToolkit, public FGCObject
{
public:
	FJavascriptEdToolkit(UJavascriptEdMode* InParent)
		: Parent(InParent)
	{}

	virtual FString GetReferencerName() const
	{
		return "FJavascriptEdToolkit";
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(Parent);
	}

	/** Initializes the geometry mode toolkit */
	virtual void Init(const TSharedPtr< class IToolkitHost >& InitToolkitHost) override
	{
		FModeToolkit::Init(InitToolkitHost);
	}

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override
	{
		return Parent->ModeId;
	}

	virtual FText GetBaseToolkitName() const override
	{
		return Parent->ModeName;
	}

	virtual class FEdMode* GetEditorMode() const override
	{
		return GLevelEditorModeTools().GetActiveMode(Parent->ModeId);
	}

	virtual TSharedPtr<class SWidget> GetInlineContent() const override
	{
		UWidget* Widget = nullptr;
		if (Parent->OnGetContent.IsBound())
		{
			Widget = Parent->OnGetContent.Execute();
		}

		if (Widget)
		{
			return SNew(SJavascriptBox).Widget(Widget)[Widget->TakeWidget()];
		}
		else
		{
			return SNullWidget::NullWidget;
		}
	}

private:
	/** Geometry tools widget */
	TSharedPtr<class SWidget> Widget;

	UJavascriptEdMode* Parent;
};

class FJavascriptEdModeInstance : public FEdMode
{
public:
	FJavascriptEdModeInstance(UJavascriptEdMode* InParent)
		: Parent(InParent)
	{}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		FEdMode::AddReferencedObjects(Collector);

		Collector.AddReferencedObject(Parent);
	}

	virtual bool UsesToolkits() const override
	{
		if (Parent->OnUsesToolkits.IsBound())
		{
			return Parent->OnUsesToolkits.Execute();
		}
		return true;
	}

	virtual bool MouseEnter(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
	{
		return Parent->OnMouseEnter.IsBound() && Parent->OnMouseEnter.Execute(FJavascriptEdViewport(ViewportClient, Viewport), x, y, this);
	}

	virtual bool MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport)
	{
		return Parent->OnMouseLeave.IsBound() && Parent->OnMouseLeave.Execute(FJavascriptEdViewport(ViewportClient, Viewport), this);
	}
	virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y)
	{
		return Parent->OnMouseMove.IsBound() && Parent->OnMouseMove.Execute(FJavascriptEdViewport(ViewportClient, Viewport), x, y, this);
	}
	virtual bool ReceivedFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
	{
		return Parent->OnReceivedFocus.IsBound() && Parent->OnReceivedFocus.Execute(FJavascriptEdViewport(ViewportClient, Viewport), this);
	}
	virtual bool LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
	{
		return Parent->OnLostFocus.IsBound() && Parent->OnLostFocus.Execute(FJavascriptEdViewport(ViewportClient, Viewport), this);
	}
	virtual bool CapturedMouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 InMouseX, int32 InMouseY)
	{
		return Parent->OnCapturedMouseMove.IsBound() && Parent->OnCapturedMouseMove.Execute(FJavascriptEdViewport(ViewportClient, Viewport), InMouseX, InMouseY, this);
	}
	virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
	{
		return Parent->OnInputKey.IsBound() && Parent->OnInputKey.Execute(FJavascriptEdViewport(ViewportClient, Viewport),Key,Event, this);
	}
	virtual bool InputAxis(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime)
	{
		return Parent->OnInputAxis.IsBound() && Parent->OnInputAxis.Execute(FJavascriptEdViewport(ViewportClient, Viewport), ControllerId, Key, Delta, DeltaTime, this);
	}
	virtual bool InputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& InDrag, FRotator& InRot, FVector& InScale)
	{
		return Parent->OnInputDelta.IsBound() && Parent->OnInputDelta.Execute(FJavascriptEdViewport(ViewportClient, Viewport), InDrag, InRot, InScale, this);
	}
	virtual bool StartTracking(FEditorViewportClient* ViewportClient, FViewport* Viewport)
	{
		return Parent->OnStartTracking.IsBound() && Parent->OnStartTracking.Execute(FJavascriptEdViewport(ViewportClient, Viewport), this);
	}
	virtual bool EndTracking(FEditorViewportClient* ViewportClient, FViewport* Viewport)
	{
		return Parent->OnEndTracking.IsBound() && Parent->OnEndTracking.Execute(FJavascriptEdViewport(ViewportClient, Viewport), this);
	}

	/** FEdMode: widget handling */
	virtual FVector GetWidgetLocation() const override
	{
		if (Parent->OnGetWidgetLocation.IsBound())
		{
			return Parent->OnGetWidgetLocation.Execute(this);
		}
		else
		{
			return FEdMode::GetWidgetLocation();
		}
	}

#define QUERY_BOOL(X,Y) virtual bool X() Y{ static FName NAME(#X); return Process(NAME,FEdMode::X()); }
	QUERY_BOOL(ShowModeWidgets, const override)
	QUERY_BOOL(AllowWidgetMove, override)
	QUERY_BOOL(ShouldDrawWidget, const override)
	QUERY_BOOL(UsesTransformWidget, const override)
	QUERY_BOOL(DisallowMouseDeltaTracking, const override)
	QUERY_BOOL(IsSnapRotationEnabled, override)
	QUERY_BOOL(AllowsViewportDragTool, const override)
	QUERY_BOOL(HandleDragDuplicate, override)

	virtual bool HandleClick(FEditorViewportClient* InViewportClient, HHitProxy *HitProxy, const FViewportClick &Click)
	{
		if (Parent->OnClick.IsBound())
		{
			FJavascriptHitProxy Proxy;
			Proxy.HitProxy = HitProxy;
			if (Parent->OnClick.Execute(FJavascriptViewportClick(&Click), Proxy, this)) return true;
		}

		return FEdMode::HandleClick(InViewportClient, HitProxy, Click);
	}

	/** Handling SelectActor */
	virtual bool Select( AActor* InActor, bool bInSelected )
	{
		if (Parent->OnSelect.IsBound())
		{
			return Parent->OnSelect.Execute(InActor, bInSelected, this);
		}
		else
		{
			return FEdMode::Select(InActor, bInSelected);
		}
	}

	/** Check to see if an actor can be selected in this mode - no side effects */
	virtual bool IsSelectionAllowed(AActor* InActor, bool bInSelection) const
	{
		if (Parent->IsSelectionAllowed.IsBound())
		{
			return Parent->IsSelectionAllowed.Execute(InActor, bInSelection, this);
		}
		else
		{
			return FEdMode::IsSelectionAllowed(InActor, bInSelection);
		}
	}

	virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
	{
		FEdMode::Render(View, Viewport, PDI);

		if (Parent->OnDraw.IsBound())
		{
			Parent->OnDraw.Execute(FJavascriptPDI(PDI), this);
		}
	}
	//void DrawGridSection(int32 ViewportLocX,int32 ViewportGridY,FVector* A,FVector* B,float* AX,float* BX,int32 Axis,int32 AlphaCase,FSceneView* View,FPrimitiveDrawInterface* PDI);

	/** Overlays the editor hud (brushes, drag tools, static mesh vertices, etc*. */
	virtual void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
	{
		FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);

		if (Parent->OnDrawHUD.IsBound())
		{
			// Create a temporary canvas if there isn't already one.
			static FName CanvasObjectName(TEXT("CanvasObject"));
			UCanvas* CanvasObject = GetCanvasByName(CanvasObjectName);
			CanvasObject->Canvas = Canvas;

			CanvasObject->Init(View->UnscaledViewRect.Width(), View->UnscaledViewRect.Height(), const_cast<FSceneView*>(View), Canvas);
			CanvasObject->ApplySafeZoneTransform();

			Parent->OnDrawHUD.Execute(CanvasObject, this);

			CanvasObject->PopSafeZoneTransform();
		}
	}

	/*virtual EAxisList::Type GetWidgetAxisToDraw(FWidget::EWidgetMode InWidgetMode) const override
	{

	}*/

	virtual bool IsCompatibleWith(FEditorModeID OtherModeID) const override
	{
		if (Parent->OnIsCompatibleWith.IsBound())
		{
			return Parent->OnIsCompatibleWith.Execute(OtherModeID);
		}
		return false;
	}

	virtual void ActorMoveNotify()
	{
		Parent->OnActorMoved.ExecuteIfBound(this);
	}

	virtual void ActorsDuplicatedNotify(TArray<AActor*>& PreDuplicateSelection, TArray<AActor*>& PostDuplicateSelection, bool bOffsetLocations)
	{
		Parent->OnActorsDuplicated.ExecuteIfBound(PreDuplicateSelection, PostDuplicateSelection, bOffsetLocations, this);
	}

	virtual void ActorSelectionChangeNotify()
	{
		Parent->OnActorSelectionChanged.ExecuteIfBound(this);
	}

	virtual void ActorPropChangeNotify()
	{
		Parent->OnActorPropChanged.ExecuteIfBound(this);
	}

	virtual void MapChangeNotify()
	{
		Parent->OnMapChanged.ExecuteIfBound(this);
	}

	virtual EEditAction::Type GetActionEditDuplicate() override { return GetAction(FName("EditDuplicate")); }
	virtual EEditAction::Type GetActionEditDelete() override { return GetAction(FName("EditDelete")); }
	virtual EEditAction::Type GetActionEditCut() override { return GetAction(FName("EditCut")); }
	virtual EEditAction::Type GetActionEditCopy() override { return GetAction(FName("EditCopy")); }
	virtual EEditAction::Type GetActionEditPaste() override { return GetAction(FName("EditPaste")); }
	virtual bool ProcessEditDuplicate() { return Process(FName("EditDuplicate")); }
	virtual bool ProcessEditDelete() { return Process(FName("EditDelete")); }
	virtual bool ProcessEditCut() { return Process(FName("EditCut")); }
	virtual bool ProcessEditCopy() { return Process(FName("EditCopy")); }
	virtual bool ProcessEditPaste() { return Process(FName("EditPaste")); }

	EEditAction::Type GetAction(FName Request)
	{
		return Parent->OnGetAction.IsBound() ? (EEditAction::Type)(Parent->OnGetAction.Execute(Request,this)) : EEditAction::Skip;
	}

	bool Process(FName Request,bool bDefaultValue = false) const
	{
		if (Parent->OnProcess.IsBound())
		{
			return Parent->OnProcess.Execute(Request, this);
		}
		else
		{
			return bDefaultValue;
		}
	}

	virtual void Enter() override
	{
		FEdMode::Enter();

		Process(FName("Enter"));

		if (!Toolkit.IsValid() && UsesToolkits())
		{
			Toolkit = MakeShareable(new FJavascriptEdToolkit(Parent));
			Toolkit->Init(Owner->GetToolkitHost());
		}
	}

	virtual void Exit() override
	{
		Process(FName("Exit"));

		if (Toolkit.IsValid())
		{
			FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
			Toolkit.Reset();
		}

		FEdMode::Exit();
	}

	UJavascriptEdMode* Parent;
};

class FJavascriptEdModeFactory : public IEditorModeFactory, public FGCObject
{
public:
	FJavascriptEdModeFactory(UJavascriptEdMode* InParent)
		: Parent(InParent)
	{}

	virtual FString GetReferencerName() const
	{
		return "FJavascriptEdModeFactory";
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(Parent);
	}

	virtual void OnSelectionChanged(FEditorModeTools& Tools, UObject* ItemUndergoingChange) const override
	{
		if (Parent->OnSelectionChanged.IsBound())
		{
			FJavascriptEditorModeTools js(&Tools);
			Parent->OnSelectionChanged.Execute(js, ItemUndergoingChange);
		}
	}

	virtual FEditorModeInfo GetModeInfo() const
	{
		return FEditorModeInfo(Parent->ModeId, Parent->ModeName, Parent->SlateIcon, Parent->bVisible, Parent->PriorityOrder);
	}

	virtual TSharedRef<FEdMode> CreateMode() const override
	{
		return MakeShareable(new FJavascriptEdModeInstance(Parent));
	}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
	virtual UEdMode* CreateScriptableMode() const override
	{
		return nullptr;
	}

	virtual bool ForScriptableMode() const override
	{
		return false;
	}
#endif

	UJavascriptEdMode* Parent;
};

void UJavascriptEdMode::Register()
{
	FEditorModeRegistry::Get().RegisterMode(ModeId, MakeShareable(new FJavascriptEdModeFactory(this)));
}

void UJavascriptEdMode::Unregister()
{
	FEditorModeRegistry::Get().UnregisterMode(ModeId);
}

#endif

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
