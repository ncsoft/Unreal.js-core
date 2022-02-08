// Copyright Epic Games, Inc. All Rights Reserved.

#include "JavascriptGraphEdNode_Comment.h"
#include "Layout/SlateRect.h"

#include "GraphEditorSettings.h"
#include "Kismet2/Kismet2NameValidators.h"

#define LOCTEXT_NAMESPACE "JavascriptEdGraphNode_Comment"

/////////////////////////////////////////////////////
// UJavascriptGraphEdNode_Comment

UJavascriptGraphEdNode_Comment::UJavascriptGraphEdNode_Comment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeWidth = 400.0f;
	NodeHeight = 100.0f;
	FontSize = 18;
	CommentColor = FLinearColor::White;
	bColorCommentBubble = false;
	MoveMode = ECommentBoxMode::GroupMovement;

	bCommentBubblePinned = true;
	bCommentBubbleVisible = true;
	bCommentBubbleVisible_InDetailsPanel = true;
	bCanResizeNode = true;
	bCanRenameNode = true;
	CommentDepth = -1;
}

void UJavascriptGraphEdNode_Comment::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UJavascriptGraphEdNode_Comment* This = CastChecked<UJavascriptGraphEdNode_Comment>(InThis);
	for (auto It = This->NodesUnderComment.CreateIterator(); It; ++It)
	{
		Collector.AddReferencedObject(*It, This);
	}

	Super::AddReferencedObjects(InThis, Collector);
}

void UJavascriptGraphEdNode_Comment::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UJavascriptGraphEdNode_Comment, bCommentBubbleVisible_InDetailsPanel))
	{
		bCommentBubbleVisible = bCommentBubbleVisible_InDetailsPanel;
		bCommentBubblePinned = bCommentBubbleVisible_InDetailsPanel;
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

FText UJavascriptGraphEdNode_Comment::GetTooltipText() const
{
	if (CachedTooltip.IsOutOfDate(this))
	{
		CachedTooltip.SetCachedText(FText::Format(NSLOCTEXT("K2Node", "CommentBlock_Tooltip", "Comment:\n{0}"), FText::FromString(NodeComment)), this);
	}
	return CachedTooltip;
}

FString UJavascriptGraphEdNode_Comment::GetDocumentationLink() const
{
	return TEXT("Shared/GraphNodes/Common");
}

FString UJavascriptGraphEdNode_Comment::GetDocumentationExcerptName() const
{
	return TEXT("UJavascriptGraphEdNode_Comment");
}

FSlateIcon UJavascriptGraphEdNode_Comment::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FLinearColor::White;

	static FSlateIcon Icon("EditorStyle", "GraphEditor.Comment_16x");
	return Icon;
}

FText UJavascriptGraphEdNode_Comment::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::MenuTitle)
	{
		return NSLOCTEXT("K2Node", "NoComment_ListTitle", "Add Comment...");
	}
	else if (TitleType == ENodeTitleType::ListView)
	{
		return NSLOCTEXT("K2Node", "CommentBlock_ListTitle", "Comment");
	}

	return FText::FromString(NodeComment);
}

FText UJavascriptGraphEdNode_Comment::GetPinNameOverride(const UEdGraphPin& Pin) const
{
	return GetNodeTitle(ENodeTitleType::ListView);
}

FLinearColor UJavascriptGraphEdNode_Comment::GetNodeCommentColor() const
{
	// Only affects the 'zoomed out' comment bubble color, not the box itself
	return (bColorCommentBubble)
		? CommentColor
		: FLinearColor::White;
}

void UJavascriptGraphEdNode_Comment::ResizeNode(const FVector2D& NewSize)
{
	if (bCanResizeNode)
	{
		NodeHeight = NewSize.Y;
		NodeWidth = NewSize.X;
	}
}

void UJavascriptGraphEdNode_Comment::AddNodeUnderComment(UObject* Object)
{
	if (UJavascriptGraphEdNode_Comment* ChildComment = Cast<UJavascriptGraphEdNode_Comment>(Object))
	{
		CommentDepth = FMath::Min(CommentDepth, ChildComment->CommentDepth - 1);
	}
	NodesUnderComment.Add(Object);
}

void UJavascriptGraphEdNode_Comment::ClearNodesUnderComment()
{
	NodesUnderComment.Empty();
}

void UJavascriptGraphEdNode_Comment::SetBounds(const class FSlateRect& Rect)
{
	NodePosX = Rect.Left;
	NodePosY = Rect.Top;

	FVector2D Size = Rect.GetSize();
	NodeWidth = Size.X;
	NodeHeight = Size.Y;
}

const FCommentNodeSet& UJavascriptGraphEdNode_Comment::GetNodesUnderComment() const
{
	return NodesUnderComment;
}

void UJavascriptGraphEdNode_Comment::OnRenameNode(const FString& NewName)
{
	NodeComment = NewName;
	CachedTooltip.MarkDirty();
}

TSharedPtr<class INameValidatorInterface> UJavascriptGraphEdNode_Comment::MakeNameValidator() const
{
	// Comments can be duplicated, etc...
	return MakeShareable(new FDummyNameValidator(EValidatorResult::Ok));
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
