#pragma once

#include "JavascriptUMGLibrary.h"
#include "Styling/SlateTypes.h"
#include "JavascriptMenuContext.generated.h"

UENUM(BlueprintType)
namespace EJavasrciptUserInterfaceActionType
{
	enum Type
	{
		/** An action which should not be associated with a user interface action */
		None,

		/** Momentary buttons or menu items.  These support enable state, and execute a delegate when clicked. */
		Button,

		/** Toggleable buttons or menu items that store on/off state.  These support enable state, and execute a delegate when toggled. */
		ToggleButton,

		/** Radio buttons are similar to toggle buttons in that they are for menu items that store on/off state.  However they should be used to indicate that menu items in a group can only be in one state */
		RadioButton,

		/** Similar to Button but will display a readonly checkbox next to the item. */
		Check,

		/** Similar to Button but has the checkbox area collapsed */
		CollapsedButton
	};
}

/**
 * 
 */
UCLASS()
class JAVASCRIPTUMG_API UJavascriptMenuContext : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FBoolDelegate);
	DECLARE_DYNAMIC_DELEGATE(FExecuteAction);
	DECLARE_DYNAMIC_DELEGATE_RetVal(ECheckBoxState, FActionCheckStateDelegate);

	UPROPERTY(EditAnywhere, Category = "Javascript | UMG")
	FText Description;

	UPROPERTY(EditAnywhere, Category = "Javascript | UMG")
	FText ToolTip;

	UPROPERTY(EditAnywhere, Category = "Javascript | UMG")
	FJavascriptSlateIcon Icon;

	UPROPERTY(EditAnywhere, Category = "Javascript | UMG")
	TEnumAsByte<EJavasrciptUserInterfaceActionType::Type> ActionType;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FBoolDelegate OnCanExecute;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FExecuteAction OnExecute;
	
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FActionCheckStateDelegate OnGetActionCheckState;

	bool Public_CanExecute();
	void Public_Execute();
	ECheckBoxState Public_GetActionCheckState();
};
