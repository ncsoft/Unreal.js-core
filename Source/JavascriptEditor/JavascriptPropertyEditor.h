#pragma once

#include "Components/Widget.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "JavascriptPropertyEditor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPropertyEditorPropertyChanged, FName, PropertyName, FName, MemberPropertyName);

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
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "PropertyEditor")
	void Construct();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "PropertyEditor")
	void Destruct();

	UFUNCTION(BlueprintCallable, Category = "PropertyEditor")
	void SetObject(UObject* Object, bool bForceRefresh);

	UFUNCTION(BlueprintCallable, Category = "PropertyEditor")
	void SetObjects(TArray<UObject*> Objects, bool bForceRefresh, bool bOverrideLock);

	UFUNCTION(BlueprintCallable, Category = "PropertyEditor")
	void ForceRefresh();

	// Note the comment in implementation of BuildPropertyPathMap for the reason
	// why the parameter PropertyPaths is array of strings instead of unique string.
	UFUNCTION(BlueprintNativeEvent, Category = "PropertyEditor")
	bool IsPropertyReadOnly(const FString& PropertyName, const FString& ParentPropertyName, const TArray<FString>& PropertyPaths);

	// Note the comment in implementation of BuildPropertyPathMap for the reason
	// why the parameter PropertyPaths is array of strings instead of unique string.
	UFUNCTION(BlueprintNativeEvent, Category = "PropertyEditor")
	bool IsPropertyVisible(const FString& PropertName, const FString& ParentPropertyName, const TArray<FString>& PropertyPaths);

	UPROPERTY(BlueprintAssignable, Category = "PropertyEditor")
	FPropertyEditorPropertyChanged OnChange;

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
	bool bEnablePropertyPath;

	UPROPERTY(BlueprintReadWrite, Category = "PropertyEditor")
	EPropertyEditorNameAreaSettings NameAreaSettings;

	TArray<FWeakObjectPtr> ObjectsToInspect;

public:
	// UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// End of UVisual interface

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void OnWidgetRebuilt() override;
	// End of UWidget interface

protected:
	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);
	bool NativeIsPropertyReadOnly(const FPropertyAndParent& InPropertyAndParent);
	bool NativeIsPropertyVisible(const FPropertyAndParent& InPropertyAndParent);

	void BuildPropertyPathMap(UStruct* InPropertyRootType);

protected:
	TSharedPtr<class IDetailsView> View;

	TWeakObjectPtr<UStruct> PropertyRootType;
	TMap<FProperty*, TArray<FString>> PropertyPathMap;

	static const FString EmptyString;
	static const TArray<FString> EmptyStringArray;
#endif	
};
