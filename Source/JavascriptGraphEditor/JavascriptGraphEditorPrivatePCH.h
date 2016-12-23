#pragma once

#include "CoreUObject.h"
#include "Engine.h"
#include "UnrealEd.h"
#include "ScopedTransaction.h"

#include "Kismet2/BlueprintEditorUtils.h"

#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphUtilities.h"
#include "GraphEditorSettings.h"

#include "GenericCommands.h"

#include "SGraphNode.h"
#include "SGraphPin.h"
#include "SInlineEditableTextBlock.h"
#include "SCommentBubble.h"

#include "SDockTab.h"
#include "SNodePanel.h"
#include "EdGraph/EdGraphSchema.h"

#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/AssetEditorManager.h"
#include "Editor/EditorWidgets/Public/ITransportControl.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.
#include "IJavascriptGraphEditor.h"

#define LOG_WARNING(FMT, ...) UE_LOG(JavascriptGraphEditor, Warning, (FMT), ##__VA_ARGS__)
