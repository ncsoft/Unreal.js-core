// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Textures/SlateIcon.h"
#include "EdGraph/EdGraphNodeUtils.h"
#include "JavascriptGraphEdNode.h"
#include "JavascriptGraphEdNode_Comment.generated.h"

class INameValidatorInterface;
struct Rect;

typedef TArray<class UObject*> FCommentNodeSet;

UCLASS(MinimalAPI)
class UJavascriptGraphEdNode_Comment : public UJavascriptGraphEdNode
{
public:
	GENERATED_BODY()

public:
	UJavascriptGraphEdNode_Comment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Color to style comment with */
	UPROPERTY(EditAnywhere, Category = Comment)
	FLinearColor CommentColor;

	/** Size of the text in the comment box */
	UPROPERTY(EditAnywhere, Category = Comment, meta = (ClampMin = 1, ClampMax = 1000))
	int32 FontSize;

	/** Whether to show a zoom-invariant comment bubble when zoomed out (making the comment readable at any distance). */
	UPROPERTY(EditAnywhere, Category = Comment, meta = (DisplayName = "Show Bubble When Zoomed"))
	uint32 bCommentBubbleVisible_InDetailsPanel : 1;

	/** Whether to use Comment Color to color the background of the comment bubble shown when zoomed out. */
	UPROPERTY(EditAnywhere, Category = Comment, meta = (DisplayName = "Color Bubble", EditCondition = bCommentBubbleVisible_InDetailsPanel))
	uint32 bColorCommentBubble : 1;

	/** Whether the comment should move any fully enclosed nodes around when it is moved */
	UPROPERTY(EditAnywhere, Category = Comment)
	TEnumAsByte<ECommentBoxMode::Type> MoveMode;

	/** comment Depth */
	UPROPERTY()
	int32 CommentDepth;

public:

	//~ Begin UObject Interface
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	//~ Begin UEdGraphNode Interface
	virtual void AllocateDefaultPins() override {}
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeCommentColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual bool ShouldOverridePinNames() const override { return true; }
	virtual FText GetPinNameOverride(const UEdGraphPin& Pin) const override;
	virtual void ResizeNode(const FVector2D& NewSize) override;
	virtual void OnRenameNode(const FString& NewName) override;
	virtual TSharedPtr<class INameValidatorInterface> MakeNameValidator() const override;
	virtual FString GetDocumentationLink() const override;
	virtual FString GetDocumentationExcerptName() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface

	/** Add a node that will be dragged when this comment is dragged */
	void AddNodeUnderComment(UObject* Object);

	/** Clear all of the nodes which are currently being dragged with this comment */
	void ClearNodesUnderComment();

	/** Set the Bounds for the comment node */
	void SetBounds(const class FSlateRect& Rect);

	/** Return the set of nodes underneath the comment */
	const FCommentNodeSet& GetNodesUnderComment() const;

private:
	/** Nodes currently within the region of the comment */
	FCommentNodeSet	NodesUnderComment;

	/** Constructing FText strings can be costly, so we cache the node's tooltip */
	FNodeTextCache CachedTooltip;
};

