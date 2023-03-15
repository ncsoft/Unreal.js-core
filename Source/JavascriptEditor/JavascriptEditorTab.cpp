PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#include "JavascriptEditorTab.h"
#if WITH_EDITOR
#include "Components/Button.h"
#include "Widgets/Layout/SSpacer.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#endif

UJavascriptEditorTab::UJavascriptEditorTab(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
#if WITH_EDITOR
, bIsNomad(true), bRegistered(false)
#endif
{	
}

#if WITH_EDITOR
TSharedPtr<SDockTab> UJavascriptEditorTab::MajorTab = nullptr;

void UJavascriptEditorTab::BeginDestroy()
{
	Super::BeginDestroy();

	Discard();
}

void UJavascriptEditorTab::Commit()
{
	Discard();

	IJavascriptEditorModule::Get().AddExtension(this);
	
	bRegistered = true;
}

void UJavascriptEditorTab::Discard()
{
	if (bRegistered)
	{
		IJavascriptEditorModule::Get().RemoveExtension(this);
	}

	bRegistered = false;
}

void UJavascriptEditorTab::ForceCommit()
{
	const TSharedRef<FGlobalTabmanager>& GlobalTabManager = FGlobalTabmanager::Get();
	if (bIsNomad && GlobalTabManager->HasTabSpawner(TabId))
	{
		GlobalTabManager->UnregisterNomadTabSpawner(TabId);
	}

	Commit();
}

UWidget* UJavascriptEditorTab::TakeWidget(UObject* Context)
{
	if (OnSpawnTab.IsBound())
	{
		UWidget* Widget = OnSpawnTab.Execute(Context);
		if (Widget) return Widget;
	}
	return NewObject<UButton>();
}

void UJavascriptEditorTab::TabActivatedCallback(TSharedRef<SDockTab> Tab, ETabActivationCause Cause)
{
	if (OnTabActivatedCallback.IsBound())
	{
		OnTabActivatedCallback.Execute(Tab->GetLayoutIdentifier().ToString(), TEnumAsByte<EJavasriptTabActivationCause::Type>((uint8)Cause));
	}
}

struct FJavascriptEditorTabTracker : public FGCObject
{
	TArray<UJavascriptEditorTab*> Spawners;
	TArray<UWidget*> Widgets;
	TArray<TWeakPtr<SDockTab>> Tabs;

	virtual FString GetReferencerName() const
	{
		return "FJavascriptEditorTabTracker";
	}

	void OnTabClosed(UWidget* Widget)
	{
		for (int Index = Tabs.Num() - 1; Index >= 0; --Index)
		{
			if (Widgets[Index] == Widget)
			{
				RemoveIndex(Index);
				break;
			}
		}
	}

	void OnTabClosed(TSharedRef<SDockTab> Tab)
	{
		Tab->SetContent(SNew(SSpacer));

		for (int Index = Tabs.Num() - 1; Index >= 0; --Index)
		{
			if (Tabs[Index] == Tab)
			{
				RemoveIndex(Index);
				break;
			}
		}		
	}

	void RemoveIndex(int Index)
	{
		if (Tabs[Index].IsValid())
		{
			Tabs[Index].Pin()->SetContent(SNew(SSpacer));
		}
		Spawners[Index]->OnCloseTab.ExecuteIfBound(Widgets[Index]);

		UWidget* ReleaseWidget = Widgets[Index];

		Spawners.RemoveAt(Index, 1);
		Tabs.RemoveAt(Index, 1);
		Widgets.RemoveAt(Index, 1);

		// Unlike general usage, widgets used to deliver slate to a jstab may not be descendants of UserWidget, so the tab must release them explicitly.
		if (ReleaseWidget && !ReleaseWidget->IsA(UUserWidget::StaticClass()))
		{
			ReleaseWidget->ReleaseSlateResources(true);
		}
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		for (int Index = Tabs.Num() - 1; Index >= 0; --Index)
		{
			if (!Tabs[Index].IsValid())
			{
				RemoveIndex(Index);
			}
		}

		Collector.AddReferencedObjects(Widgets);
		Collector.AddReferencedObjects(Spawners);
	}

	bool bInit{ false };

	void MaybeInit()
	{
		if (bInit) return;

		bInit = true;

		FEditorDelegates::OnShutdownPostPackagesSaved.AddLambda([this]() {
			FEditorScriptExecutionGuard ScriptGuard;

			while (Tabs.Num())
			{
				RemoveIndex(Tabs.Num() - 1);
			}
		});
	}

	void Add(UJavascriptEditorTab* Spawner, UWidget* Widget, TWeakPtr<SDockTab> Tab)
	{
		MaybeInit();

		Spawners.Add(Spawner);
		Widgets.Add(Widget);
		Tabs.Add(Tab);
	}

	TSharedPtr<SDockTab> Find(UWidget* Widget)
	{
		for (int Index = Widgets.Num() - 1; Index >= 0; --Index)
		{
			for (auto p = Widget; p; p = p->GetParent())
			{
				if (p == Widgets[Index])
				{
					return Tabs[Index].Pin();
				}
			}
		}

		return TSharedPtr<SDockTab>();
	}
} GEditorTabTracker;

TSharedPtr<SDockTab> UJavascriptEditorTab::FindDocktab(UWidget* Widget)
{
	// under construction?
	if (MajorTab.IsValid()) return MajorTab;

	return GEditorTabTracker.Find(Widget);
}

void UJavascriptEditorTab::CloseTab(UWidget* Widget)
{
	TSharedPtr<SDockTab> ThisTabPtr = ThisTab.Pin();
	if (ThisTabPtr.IsValid())
	{
		ThisTabPtr->RequestCloseTab();
	}

	GEditorTabTracker.OnTabClosed(Widget);
}

void UJavascriptEditorTab::Register(TSharedRef<FTabManager> TabManager, UObject* Context, TSharedRef<FWorkspaceItem> Group)
{
	if (Icon.StyleSetName.IsNone())
	{
		Icon.StyleSetName = FAppStyle::Get().GetStyleSetName();
		Icon.StyleName = "DeviceDetails.Tabs.ProfileEditor";
	}

	const bool bGlobal = TabManager == FGlobalTabmanager::Get();

	auto Lambda = FOnSpawnTab::CreateLambda([this, Context, bGlobal](const FSpawnTabArgs& SpawnTabArgs){
		auto Widget = this->TakeWidget(Context);

		const TSharedRef<SDockTab> MajorTab = SNew(SDockTab)
			.TabRole(ETabRole(Role.GetValue()))
			.OnTabClosed_Lambda([](TSharedRef<SDockTab> ClosedTab) {
				GEditorTabTracker.OnTabClosed(ClosedTab);
			});

		GEditorTabTracker.Add(this, Widget, MajorTab);

		//auto OldTab = UJavascriptEditorTab::MajorTab;
		UJavascriptEditorTab::MajorTab = MajorTab;
		ThisTab = MajorTab;
		
		MajorTab->SetContent(Widget->TakeWidget());
		MajorTab->SetOnTabActivated(FOnTabActivatedCallback::CreateLambda([this](TSharedRef<SDockTab> Tab, ETabActivationCause Cause) {
			this->TabActivatedCallback(Tab, Cause);
		}));
		UJavascriptEditorTab::MajorTab = nullptr;

		return MajorTab;
	});	

	auto& SpawnerEntry = bIsNomad && TabManager == FGlobalTabmanager::Get() ? FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId, Lambda) : TabManager->RegisterTabSpawner(TabId, Lambda);
	SpawnerEntry
		.SetDisplayName(DisplayName)
		.SetIcon(Icon.GetSlateIcon())
		.SetGroup(Group);
}

void UJavascriptEditorTab::InsertTo(TSharedRef<FTabManager> TabManager, UObject* Context, FName PlaceholderId, FName SearchForTabId)
{
	if (Icon.StyleSetName.IsNone())
	{
		Icon.StyleSetName = FAppStyle::Get().GetStyleSetName();
		Icon.StyleName = "DeviceDetails.Tabs.ProfileEditor";
	}

	auto Widget = this->TakeWidget(Context);

	const TSharedRef<SDockTab> MajorTab = SNew(SDockTab)
		.TabRole(ETabRole(Role.GetValue()))
		.Label(DisplayName)
		.OnTabClosed_Lambda([](TSharedRef<SDockTab> ClosedTab) {
			GEditorTabTracker.OnTabClosed(ClosedTab);
		});

	MajorTab->SetTabIcon(Icon.GetSlateIcon().GetIcon());

	GEditorTabTracker.Add(this, Widget, MajorTab);

	UJavascriptEditorTab::MajorTab = MajorTab;
	ThisTab = MajorTab;

	MajorTab->SetContent(Widget->TakeWidget());
	MajorTab->SetOnTabActivated(FOnTabActivatedCallback::CreateLambda([this](TSharedRef<SDockTab> Tab, ETabActivationCause Cause) {
		this->TabActivatedCallback(Tab, Cause);
	}));
	UJavascriptEditorTab::MajorTab = nullptr;

	TSharedPtr<FTabManager::FSearchPreference> SearchPreference = MakeShareable(new FTabManager::FLiveTabSearch(SearchForTabId));
	TabManager->InsertNewDocumentTab(PlaceholderId, *SearchPreference, MajorTab);
}

void UJavascriptEditorTab::Unregister(TSharedRef<FTabManager> TabManager)
{
	if (bIsNomad && TabManager == FGlobalTabmanager::Get())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
	}
	else
	{
		TabManager->UnregisterTabSpawner(TabId);
	}	
}

void UJavascriptEditorTab::ActivateInParent()
{
	TSharedPtr<SDockTab> ThisTabPtr = ThisTab.Pin();
	if (ThisTabPtr.IsValid())
	{
		ThisTabPtr->ActivateInParent(ETabActivationCause::SetDirectly);
	}
}

void UJavascriptEditorTab::SetTabLabel(const FString& InLabelName)
{
	TSharedPtr<SDockTab> ThisTabPtr = ThisTab.Pin();
	if (ThisTabPtr.IsValid())
	{
		FText Text = FText::FromString(InLabelName);
		ThisTabPtr->SetLabel(Text);
	}
}

void UJavascriptEditorTab::Register()
{	
	Register(FGlobalTabmanager::Get(), nullptr, Group.Handle.IsValid() ? Group.Handle.ToSharedRef() : WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());
}

void UJavascriptEditorTab::Unregister()
{
	Unregister(FGlobalTabmanager::Get());
}
#endif

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
