#pragma once

#include "CoreMinimal.h"
#include "Widgets/Docking/SDockTab.h"
#include "JavascriptEditorLibrary.h"
#include "UObject/Object.h"
#include "JavascriptEditorModule.h"
#include "JavascriptEditorTab.generated.h"

UENUM()
namespace EJavascriptTabRole
{
	enum Type
	{
		MajorTab,
		PanelTab,
		NomadTab,
		DocumentTab		
	};
}

UENUM()
namespace EJavasriptTabActivationCause
{
	enum Type
	{
		UserClickedOnTab,
		SetDirectly
	};
}

class UJavascriptEditorTabManager;
/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEditorTab : public UObject, public IEditorExtension
{
	GENERATED_UCLASS_BODY()

public:	
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(UWidget*, FSpawnTab, UObject*, Context);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCloseTab, UWidget*, Widget);

	/** Invoked when a tab is activated */
	DECLARE_DELEGATE_TwoParams(FOnTabActivatedCallback, TSharedRef<SDockTab>, ETabActivationCause);
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnTabActivated, FString, TabId, TEnumAsByte<EJavasriptTabActivationCause::Type>, Cause);

#if WITH_EDITOR
	UPROPERTY()
	FSpawnTab OnSpawnTab;	

	UPROPERTY()
	FCloseTab OnCloseTab;

	UPROPERTY()
	FOnTabActivated OnTabActivatedCallback;

	UPROPERTY()
	FJavascriptWorkspaceItem Group;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FName TabId;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FText DisplayName;

	UPROPERTY(EditAnywhere, Category = "Javascript | UMG")
	FJavascriptSlateIcon Icon;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	bool bIsNomad;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	TEnumAsByte<EJavascriptTabRole::Type> Role;

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Commit();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Discard();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void ForceCommit();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void CloseTab(UWidget* Widget);

	bool bRegistered;

	UWidget* TakeWidget(UObject* Context);

	void TabActivatedCallback(TSharedRef<SDockTab> Tab, ETabActivationCause Cause);

	virtual void Register() override;
	virtual void Unregister() override;

	virtual void BeginDestroy() override;

	void Register(TSharedRef<FTabManager> TabManager, UObject* Context, TSharedRef<FWorkspaceItem> Group);
	void Unregister(TSharedRef<FTabManager> TabManager);
	void InsertTo(TSharedRef<FTabManager> TabManager, UObject* Context, FName PlaceholderId, FName SearchForTabId);

	static TSharedPtr<SDockTab> MajorTab;
	static TSharedPtr<SDockTab> FindDocktab(UWidget* Widget);
#endif
};
