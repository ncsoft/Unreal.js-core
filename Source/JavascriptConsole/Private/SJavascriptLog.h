#pragma once
#include "Misc/TextFilterExpressionEvaluator.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/STextComboBox.h"

class FJavascriptLogTextLayoutMarshaller;


/**
* A single log message for the output log, holding a message and
* a style, for color and bolding of the message.
*/
struct FLogMessage
{
	TSharedRef<FString> Message;
	ELogVerbosity::Type Verbosity;
	FName Category;
	FName Style;

	FLogMessage(const TSharedRef<FString>& NewMessage, FName NewCategory, FName NewStyle = NAME_None)
		: Message(NewMessage)
		, Verbosity(ELogVerbosity::Log)
		, Category(NewCategory)
		, Style(NewStyle)
	{
	}

	FLogMessage(const TSharedRef<FString>& NewMessage, ELogVerbosity::Type NewVerbosity, FName NewCategory, FName NewStyle = NAME_None)
		: Message(NewMessage)
		, Verbosity(NewVerbosity)
		, Category(NewCategory)
		, Style(NewStyle)
	{
	}
};

/** Expression context to test the given messages against the current text filter */
class FJSLogFilter_TextFilterExpressionContext : public ITextFilterExpressionContext
{
public:
	explicit FJSLogFilter_TextFilterExpressionContext(const FLogMessage& InMessage) : Message(&InMessage) {}

	/** Test the given value against the strings extracted from the current item */
	virtual bool TestBasicStringExpression(const FTextFilterString& InValue, const ETextFilterTextComparisonMode InTextComparisonMode) const override { return TextFilterUtils::TestBasicStringExpression(*Message->Message, InValue, InTextComparisonMode); }

	/**
	* Perform a complex expression test for the current item
	* No complex expressions in this case - always returns false
	*/
	virtual bool TestComplexExpression(const FName& InKey, const FTextFilterString& InValue, const ETextFilterComparisonOperation InComparisonOperation, const ETextFilterTextComparisonMode InTextComparisonMode) const override { return false; }

private:
	/** Message that is being filtered */
	const FLogMessage* Message;
};


/**
 * Console input box with command-completion support
 */
class SJavascriptConsoleInputBox
	: public SCompoundWidget
{

public:

	SLATE_BEGIN_ARGS( SJavascriptConsoleInputBox )
		: _SuggestionListPlacement( MenuPlacement_BelowAnchor )
		{}

		/** Where to place the suggestion list */
		SLATE_ARGUMENT( EMenuPlacement, SuggestionListPlacement )

		/** Called when a console command is executed */
		SLATE_EVENT( FSimpleDelegate, OnConsoleCommandExecuted )
	SLATE_END_ARGS()

	/** Protected console input box widget constructor, called by Slate */
	SJavascriptConsoleInputBox();

	/**
	 * Construct this widget.  Called by the SNew() Slate macro.
	 *
	 * @param	InArgs	Declaration used by the SNew() macro to construct this widget
	 */
	void Construct( const FArguments& InArgs );

	/** Returns the editable text box associated with this widget.  Used to set focus directly. */
	TSharedRef< SEditableTextBox > GetEditableTextBox()
	{
		return InputText.ToSharedRef();
	}

	/** SWidget interface */
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

protected:

	virtual bool SupportsKeyboardFocus() const override { return true; }

	// e.g. Tab or Key_Up
	virtual FReply OnPreviewKeyDown( const FGeometry& MyGeometry, const FKeyEvent& KeyEvent ) override;

	void OnFocusLost( const FFocusEvent& InFocusEvent ) override;

	/** Handles entering in a command */
	void OnTextCommitted(const FText& InText, ETextCommit::Type CommitInfo);

	void OnTextChanged(const FText& InText);

	/** Makes the widget for the suggestions messages in the list view */
	TSharedRef<ITableRow> MakeSuggestionListItemWidget(TSharedPtr<FString> Message, const TSharedRef<STableViewBase>& OwnerTable);

	void SuggestionSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
		
	void SetSuggestions(TArray<FString>& Elements, bool bInHistoryMode);

	void MarkActiveSuggestion();

	void ClearSuggestions();

	FString GetSelectionText() const;


private:

	/** Editable text widget */
	TSharedPtr< SEditableTextBox > InputText;

	/** history / auto completion elements */
	TSharedPtr< SMenuAnchor > SuggestionBox;

	/** All log messages stored in this widget for the list view */
	TArray< TSharedPtr<FString> > Suggestions;

	/** The list view for showing all log messages. Should be replaced by a full text editor */
	TSharedPtr< SListView< TSharedPtr<FString> > > SuggestionListView;

	/** Delegate to call when a console command is executed */
	FSimpleDelegate OnConsoleCommandExecuted;

	/** -1 if not set, otherwise index into Suggestions */
	int32 SelectedSuggestion;

	/** to prevent recursive calls in UI callback */
	bool bIgnoreUIUpdate; 

	TSharedPtr<STextComboBox> ContextComboBox;

	TArray<TSharedPtr<FString> > ContextArray;
	TSharedPtr<FString> TargetContext;
};


/**
* Holds information about filters
*/
struct FJavascriptLogFilter
{
	/** true to show Logs. */
	bool bShowLogs;

	/** true to show Warnings. */
	bool bShowWarnings;

	/** true to show Errors. */
	bool bShowErrors;

	/** true to allow all Log Categories */
	bool bShowAllCategories;

	/** Enable all filters by default */
	FJavascriptLogFilter() : TextFilterExpressionEvaluator(ETextFilterExpressionEvaluatorMode::BasicString)
	{
		bShowErrors = bShowLogs = bShowWarnings = bShowAllCategories = true;
	}

	/** Returns true if any messages should be filtered out */
	bool IsFilterSet() { return !bShowErrors || !bShowLogs || !bShowWarnings || TextFilterExpressionEvaluator.GetFilterType() != ETextFilterExpressionType::Empty || !TextFilterExpressionEvaluator.GetFilterText().IsEmpty(); }

	/** Checks the given message against set filters */
	bool IsMessageAllowed(const TSharedPtr<FLogMessage>& Message);

	/** Set the Text to be used as the Filter's restrictions */
	void SetFilterText(const FText& InFilterText) { TextFilterExpressionEvaluator.SetFilterText(InFilterText); }

	/** Get the Text currently being used as the Filter's restrictions */
	const FText GetFilterText() const { return TextFilterExpressionEvaluator.GetFilterText(); }

	/** Returns Evaluator syntax errors (if any) */
	FText GetSyntaxErrors() { return TextFilterExpressionEvaluator.GetFilterErrorText(); }

	const TArray<FName>& GetAvailableLogCategories() { return AvailableLogCategories; }

	/** Adds a Log Category to the list of available categories, if it isn't already present */
	void AddAvailableLogCategory(FName& LogCategory);

	/** Enables or disables a Log Category in the filter */
	void ToggleLogCategory(const FName& LogCategory);

	/** Returns true if the specified log category is enabled */
	bool IsLogCategoryEnabled(const FName& LogCategory) const;

	/** Empties the list of selected log categories */
	void ClearSelectedLogCategories();

private:
	/** Expression evaluator that can be used to perform complex text filter queries */
	FTextFilterExpressionEvaluator TextFilterExpressionEvaluator;

	/** Array of Log Categories which are available for filter -- i.e. have been used in a log this session */
	TArray<FName> AvailableLogCategories;

	/** Array of Log Categories which are being used in the filter */
	TArray<FName> SelectedLogCategories;
};


/**
 * Widget which holds a list view of logs of the program output
 * as well as a combo box for entering in new commands
 */
class SJavascriptLog 
	: public SCompoundWidget, public FOutputDevice
{

public:

	SLATE_BEGIN_ARGS( SJavascriptLog )
		: _Messages()
		{}
		
		/** All messages captured before this log window has been created */
		SLATE_ARGUMENT( TArray< TSharedPtr<FLogMessage> >, Messages )

	SLATE_END_ARGS()

	/** Destructor for output log, so we can unregister from notifications */
	~SJavascriptLog();

	/**
	 * Construct this widget.  Called by the SNew() Slate macro.
	 *
	 * @param	InArgs	Declaration used by the SNew() macro to construct this widget
	 */
	void Construct( const FArguments& InArgs );

	/**
	 * Creates FLogMessage objects from FOutputDevice log callback
	 *
	 * @param	V Message text
	 * @param Verbosity Message verbosity
	 * @param Category Message category
	 * @param OutMessages Array to receive created FLogMessage messages
	 *
	 * @return true if any messages have been created, false otherwise
	 */
	static bool CreateLogMessages( const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category, TArray< TSharedPtr<FLogMessage> >& OutMessages );

protected:

	virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category ) override;

private:
	/**
	 * Extends the context menu used by the text box
	 */
	void ExtendTextBoxMenu(FMenuBuilder& Builder);

	/**
	 * Called when delete all is selected
	 */
	void OnClearLog();

	/**
	 * Called when the user scrolls the log window vertically
	 */
	void OnUserScrolled(float ScrollOffset);

	/**
	 * Called to determine whether delete all is currently a valid command
	 */
	bool CanClearLog() const;

	/** Called when a console command is entered for this output log */
	void OnConsoleCommandExecuted();

	/** Request we immediately force scroll to the bottom of the log */
	void RequestForceScroll();

	/** Converts the array of messages into something the text box understands */
	TSharedPtr< FJavascriptLogTextLayoutMarshaller > MessagesTextMarshaller;

	/** The editable text showing all log messages */
	TSharedPtr< SMultiLineEditableTextBox > MessagesTextBox;

	/** The editable text showing all log messages */
	TSharedPtr< SSearchBox > FilterTextBox;

	/** True if the user has scrolled the window upwards */
	bool bIsUserScrolled;

private:
	/** Called by Slate when the filter box changes text. */
	void OnFilterTextChanged(const FText& InFilterText);

	/** Called by Slate when the filter text box is confirmed. */
	void OnFilterTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType);

	/** Make the "Filters" menu. */
	TSharedRef<SWidget> MakeAddFilterMenu();

	/** Make the "Categories" sub-menu. */
	void MakeSelectCategoriesSubMenu(FMenuBuilder& MenuBuilder);

	/** Toggles Verbosity "Logs" true/false. */
	void VerbosityLogs_Execute();

	/** Returns the state of Verbosity "Logs". */
	bool VerbosityLogs_IsChecked() const;

	/** Toggles Verbosity "Warnings" true/false. */
	void VerbosityWarnings_Execute();

	/** Returns the state of Verbosity "Warnings". */
	bool VerbosityWarnings_IsChecked() const;

	/** Toggles Verbosity "Errors" true/false. */
	void VerbosityErrors_Execute();

	/** Returns the state of Verbosity "Errors". */
	bool VerbosityErrors_IsChecked() const;

	/** Toggles All Categories true/false. */
	void CategoriesShowAll_Execute();

	/** Returns the state of "Show All" */
	bool CategoriesShowAll_IsChecked() const;

	/** Toggles the given category true/false. */
	void CategoriesSingle_Execute(FName InName);

	/** Returns the state of the given category */
	bool CategoriesSingle_IsChecked(FName InName) const;

	/** Forces re-population of the messages list */
	void Refresh();

public:
	/** Visible messages filter */
	FJavascriptLogFilter Filter;
};
