#pragma once

#include "Components//ContentWidget.h"
#include "Styling/SlateTypes.h"
#include "JavascriptWindow.generated.h"

class SWindow;

/** Enumeration to specify different window types for SWindows */
UENUM()
enum class EJavascriptWindowType : uint8
{
	/** Value indicating that this is a standard, general-purpose window */
	Normal,
	/** Value indicating that this is a window used for a popup menu */
	Menu,
	/** Value indicating that this is a window used for a tooltip */
	ToolTip,
	/** Value indicating that this is a window used for a notification toast */
	Notification,
	/** Value indicating that this is a window used for a cursor decorator */
	CursorDecorator
};

/** Enum to describe how to auto-center an SWindow */
UENUM()
enum class EJavascriptAutoCenter : uint8
{
	/** Don't auto-center the window */
	None,

	/** Auto-center the window on the primary work area */
	PrimaryWorkArea,

	/** Auto-center the window on the preferred work area, determined using GetPreferredWorkArea() */
	PreferredWorkArea,
};

UENUM()
enum class EJavascriptSizingRule : uint8
{
	/* The windows size fixed and cannot be resized **/
	FixedSize,

	/** The window size is computed from its content and cannot be resized by users */
	Autosized,

	/** The window can be resized by users */
	UserSized,
};

/** Enumeration to specify different transparency options for SWindows */
UENUM()
enum class EJavascriptWindowTransparency : uint8
{
	/** Value indicating that a window does not support transparency */
	None,

	/** Value indicating that a window supports transparency at the window level (one opacity applies to the entire window) */
	PerWindow,

#if ALPHA_BLENDED_WINDOWS
	/** Value indicating that a window supports per-pixel alpha blended transparency */
	PerPixel,
#endif
};


UCLASS(Experimental)
class JAVASCRIPTUMG_API UJavascriptWindow : public UContentWidget
{
	GENERATED_UCLASS_BODY()

	DECLARE_DYNAMIC_DELEGATE(FOnWindowWidgetClosed);
	DECLARE_DYNAMIC_DELEGATE(FOnWindowDeactivated);

public:		
	/** Type of this window */
	UPROPERTY()
	EJavascriptWindowType Type;

	UPROPERTY()
	FWindowStyle Style;

	UPROPERTY()
	FText Title;

	UPROPERTY()
	bool bDragAnywhere;

	UPROPERTY()
	EJavascriptAutoCenter AutoCenter;

	/** Screen-space position where the window should be initially located. */
	UPROPERTY()
	FVector2D ScreenPosition;

	/** What the initial size of the window should be. */
	UPROPERTY()
	FVector2D ClientSize;

	/** Should this window support transparency */
	UPROPERTY()
	EJavascriptWindowTransparency SupportsTransparency;

	UPROPERTY()
	float InitialOpacity;

	/** Is the window initially maximized */
	UPROPERTY()
	bool IsInitiallyMaximized;

	UPROPERTY()
	EJavascriptSizingRule SizingRule;

	/** True if this should be a 'pop-up' window */
	UPROPERTY()
	bool IsPopupWindow;

	/** Should this window be focused immediately after it is shown? */
	UPROPERTY()
	bool FocusWhenFirstShown;

	/** Should this window be activated immediately after it is shown? */
	UPROPERTY()
	bool ActivateWhenFirstShown;

	/** Use the default os look for the border of the window */
	UPROPERTY()
	bool UseOSWindowBorder;

	/** Does this window have a close button? */
	UPROPERTY()
	bool HasCloseButton;

	/** Can this window be maximized? */
	UPROPERTY()
	bool SupportsMaximize;

	/** Can this window be minimized? */
	UPROPERTY()
	bool SupportsMinimize;

	/** True if we should initially create a traditional title bar area.  If false, the user must embed the title
	area content into the window manually, taking into account platform-specific considerations!  Has no
	effect for certain types of windows (popups, tool-tips, etc.) */
	UPROPERTY()
	bool CreateTitleBar;

	/** If the window appears off screen or is too large to safely fit this flag will force realistic
	constraints on the window and bring it back into view. */
	UPROPERTY()
	bool SaneWindowPlacement;

	/** The padding around the edges of the window applied to it's content. */
	UPROPERTY()
	FMargin LayoutBorder;

	/** The margin around the edges of the window that will be detected as places the user can grab to resize the window. */
	UPROPERTY()
	FMargin UserResizeBorder;

	/** Sets the delegate to execute right before the window is closed */
	UPROPERTY()
	FOnWindowWidgetClosed OnWindowClosed;

	UPROPERTY()
	FOnWindowDeactivated OnWindowDeactivated;

	/** True if this window should always be on top of all other windows */
	UPROPERTY()
	bool IsTopmostWindow;

	void OnWindowDeactivatedEvent();

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void MoveWindowTo(FVector2D NewPosition);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void ReshapeWindow(FVector2D NewPosition, FVector2D NewSize);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void Resize(FVector2D NewSize);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void FlashWindow();
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void BringToFront();
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void RequestDestroyWindow();
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void DestroyWindowImmediately();
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void ShowWindow();
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void HideWindow();
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void EnableWindow(bool bEnable);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void SetOpacity(const float InOpacity);

	TSharedPtr<SWindow> GetSlatePtr()
	{
		return WeakWindow.Pin();
	}

protected:
	TWeakPtr<SWindow> WeakWindow;
	bool bIsCloseRequested;
};
