#include "JavascriptEditorTabManager.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "JavascriptTabManager"

#if WITH_EDITOR
class JAVASCRIPTEDITOR_API SPrimaryDockingArea : public SBox
{
public:
	SLATE_BEGIN_ARGS(SPrimaryDockingArea)
	{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SBox::Construct(SBox::FArguments());
	}
	
	TSharedPtr<FTabManager> TabManager;
	TWeakPtr<SDockTab> DockTab;
};
#endif

UJavascriptEditorTabManager::UJavascriptEditorTabManager(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

#if WITH_EDITOR	
void UJavascriptEditorTabManager::Setup(TSharedRef<SBox> Box)
{	
	auto DockArea = StaticCastSharedRef<SPrimaryDockingArea>(Box);

	TSharedRef<SDockTab> ConstructUnderMajorTab = DockArea->DockTab.Pin().ToSharedRef();
	TSharedPtr<SWindow> ConstructUnderWindow;

	TabManager = FGlobalTabmanager::Get()->NewTabManager(ConstructUnderMajorTab);
	auto AppMenuGroup = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("DeviceManagerMenuGroupName", "Device Manager"));

	TabManager->UnregisterAllTabSpawners();

	for (auto Tab : Tabs)
	{
		Tab->Register(TabManager.ToSharedRef(), nullptr, AppMenuGroup);
	}

	auto CachedLayout = FTabManager::FLayout::NewFromString(Layout);		

	if (CachedLayout.IsValid())
	{
		Box->SetContent(
			TabManager->RestoreFrom(CachedLayout.ToSharedRef(), ConstructUnderWindow).ToSharedRef()
			);
	}	

	DockArea->TabManager = TabManager;
}

TSharedRef<SWidget> UJavascriptEditorTabManager::RebuildWidget()
{	
	// Tab manager requires a parent (SDocktab)
	auto DockTab = UJavascriptEditorTab::FindDocktab(this);
	if (DockTab.IsValid())
	{
		auto PrimaryArea = SNew(SPrimaryDockingArea);
		PrimaryArea->DockTab = DockTab;

		Setup(PrimaryArea);

		return PrimaryArea;
	}
	else
	{
		return SNew(SButton);
	}	
}

void UJavascriptEditorTabManager::InsertNewTab(FName PlaceholderId, FName SearchForTabId, UJavascriptEditorTab* NewTab)
{
	if (TabManager.IsValid())
	{
		if (NewTab)
		{
			NewTab->InsertTo(TabManager.ToSharedRef(), nullptr, PlaceholderId, SearchForTabId);
		}
	}
}

void UJavascriptEditorTabManager::InvokeTab(FName SearchForTabId)
{
	if (TabManager.IsValid())
	{
		TabManager->TryInvokeTab(SearchForTabId);
	}
}

#endif

#undef LOCTEXT_NAMESPACE