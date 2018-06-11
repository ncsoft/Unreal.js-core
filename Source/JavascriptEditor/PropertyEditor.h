#pragma once

#include "Widget.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "PropertyEditor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPropertyEditorParameterChanged, FName, ParameterName);

UENUM()
enum class EPropertyEditorNameAreaSettings : uint8
{
	/** The name area should never be displayed */
	HideNameArea,
	/** All object types use name area */
	ObjectsUseNameArea,
	/** Only Actors use name area */
	ActorsUseNameArea,
	/** Components and actors use the name area. Components will display their actor owner as the name */
	ComponentsAndActorsUseNameArea,
};
/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UPropertyEditor : public UWidget
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "PropertyEditor")
	void SetObject(UObject* Object, bool bForceRefresh);

	UFUNCTION(BlueprintCallable, Category = "PropertyEditor")
	void SetObjects(TArray<UObject*> Objects, bool bForceRefresh, bool bOverrideLock);

	UPROPERTY(BlueprintAssignable, Category = "PropertyEditor")
	FPropertyEditorParameterChanged OnChange;

	UPROPERTY(BlueprintReadWrite, Category = "PropertyEditor")
	bool bUpdateFromSelection;

	UPROPERTY(BlueprintReadWrite, Category = "PropertyEditor")
	bool bLockable;

	UPROPERTY(BlueprintReadWrite, Category = "PropertyEditor")
	bool bAllowSearch;
	
	UPROPERTY(BlueprintReadWrite, Category = "PropertyEditor")
	bool bHideSelectionTip;

	UPROPERTY(BlueprintReadWrite, Category = "PropertyEditor")
	bool bReadOnly;

	UPROPERTY(BlueprintReadWrite, Category = "PropertyEditor")
	EPropertyEditorNameAreaSettings NameAreaSettings;
	
	TArray<FWeakObjectPtr> ObjectsToInspect;

public:	
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

protected:
	TSharedPtr<class IDetailsView> View;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);
#endif	
};
