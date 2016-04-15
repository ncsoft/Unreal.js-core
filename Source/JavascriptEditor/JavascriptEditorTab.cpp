#include "JavascriptEditor.h"

#include "JavascriptEditorTab.h"
#include "WorkspaceMenuStructureModule.h"

UJavascriptEditorTab::UJavascriptEditorTab(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
#if WITH_EDITOR
, bRegistered(false), bIsNomad(true)
#endif
{	
}

#if WITH_EDITOR
TSharedPtr<SDockTab> UJavascriptEditorTab::MajorTab;

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

struct FHackFindDocktab
{
	UWidget* Widget;
	TWeakPtr<SDockTab> DockTab;
};

TArray<FHackFindDocktab> GHackFindDocktabs;

TSharedPtr<SDockTab> UJavascriptEditorTab::FindDocktab(UWidget* Widget)
{
	if (MajorTab.IsValid()) return MajorTab;

	for (auto& entry : GHackFindDocktabs)
	{
		for (auto p = Widget; p; p = p->GetParent())
		{
			if (p == entry.Widget)
			{
				return entry.DockTab.Pin();
			}
		}
	}

	return TSharedPtr<SDockTab>();
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

struct FJavascriptEditorTabTracker : public FGCObject
{
	TArray<UJavascriptEditorTab*> Spawners;
	TArray<UWidget*> Widgets;
	TArray<TWeakPtr<SDockTab>> Tabs;

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

		Spawners.RemoveAt(Index, 1);
		Tabs.RemoveAt(Index, 1);
		Widgets.RemoveAt(Index, 1);
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

	void Add(UJavascriptEditorTab* Spawner, UWidget* Widget, TWeakPtr<SDockTab> Tab)
	{
		Spawners.Add(Spawner);
		Widgets.Add(Widget);
		Tabs.Add(Tab);
	}
} GEditorTabTracker;

void UJavascriptEditorTab::CloseTab(UWidget* Widget)
{
	GEditorTabTracker.OnTabClosed(Widget);
}

void UJavascriptEditorTab::Register(TSharedRef<FTabManager> TabManager, UObject* Context, TSharedRef<FWorkspaceItem> Group)
{
	FSlateIcon Icon(FEditorStyle::GetStyleSetName(), "DeviceDetails.Tabs.ProfileEditor");

	const bool bGlobal = TabManager == FGlobalTabmanager::Get();

	auto Lambda = FOnSpawnTab::CreateLambda([this, Context, bGlobal](const FSpawnTabArgs& SpawnTabArgs){
		auto Widget = this->TakeWidget(Context);

		const TSharedRef<SDockTab> MajorTab = SNew(SDockTab)
			.TabRole(ETabRole(Role.GetValue()))
			.OnTabClosed_Lambda([](TSharedRef<SDockTab> ClosedTab) {
				GEditorTabTracker.OnTabClosed(ClosedTab);
			});

		GEditorTabTracker.Add(this, Widget, MajorTab);

		auto OldTab = UJavascriptEditorTab::MajorTab;
		UJavascriptEditorTab::MajorTab = MajorTab;		 
		
		MajorTab->SetContent(Widget->TakeWidget());
		auto entry = new(GHackFindDocktabs)FHackFindDocktab;
		entry->Widget = Widget;
		entry->DockTab = MajorTab;
		
		UJavascriptEditorTab::MajorTab = OldTab;

		SpawnedTabs.Add(MajorTab);

		return MajorTab;
	});	

	auto& SpawnerEntry = bIsNomad && TabManager == FGlobalTabmanager::Get() ? FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabId, Lambda) : TabManager->RegisterTabSpawner(TabId, Lambda);
	SpawnerEntry
		.SetDisplayName(DisplayName)
		.SetIcon(Icon)
		.SetGroup(Group);	
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

void UJavascriptEditorTab::Register()
{
	Register(FGlobalTabmanager::Get(), nullptr, WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());
}

void UJavascriptEditorTab::Unregister()
{
	Unregister(FGlobalTabmanager::Get());
}

void UJavascriptEditorTab::Refresh()
{
	for (auto& tab : SpawnedTabs)
	{		
		if (tab.IsValid())
		{
			tab.Pin()->RequestCloseTab();
			SpawnedTabs.Empty();
			FGlobalTabmanager::Get()->InvokeTab(TabId);
			return;
		}
	}

	SpawnedTabs.Empty();	
}
#endif