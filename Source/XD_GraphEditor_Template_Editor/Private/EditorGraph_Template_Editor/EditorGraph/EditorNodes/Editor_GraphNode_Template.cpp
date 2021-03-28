// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/EditorGraph/EditorNodes/Editor_GraphNode_Template.h"
#include <Framework/Commands/GenericCommands.h>
#include <ToolMenu.h>
#include <ToolMenuSection.h>
#include <GraphEditorActions.h>

#include "EditorGraph_Template/Nodes/BP_GraphNode_Template.h"
#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorGraph_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/SlateWidgets/SGraphNode_Template.h"

#define LOCTEXT_NAMESPACE "Editor_GraphNode_Template"

UEditor_GraphNode_Template::UEditor_GraphNode_Template(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer){}

const FName DefualtPinType("Default");
const FName InPinName("InPin");
const FName OutPinName("OutPin");

TSharedPtr<SGraphNode> UEditor_GraphNode_Template::CreateVisualWidget()
{
	SlateNode= SNew(SGraphNode_Template, this);
	return SlateNode;
}

void UEditor_GraphNode_Template::UpdateVisualNode()
{
	if (SlateNode.IsValid())
	{
		SlateNode->UpdateGraphNode();
	}
}

void UEditor_GraphNode_Template::SaveNodesAsChildren(TArray<UEdGraphNode*>& Children)
{
    for (UEdGraphNode* Child : Children)
    {
        UEditor_GraphNode_Template* Node = Cast<UEditor_GraphNode_Template>(Child);
        if (Node)
            BP_Node_Template->LinkArgumentNodeAsChild(Node->BP_Node_Template);
    }
}

bool UEditor_GraphNode_Template::HasOutputPins()
{
	return BP_Node_Template ? BP_Node_Template->HasOutputPins() : true;
}

bool UEditor_GraphNode_Template::HasInputPins()
{
	return BP_Node_Template ? BP_Node_Template->HasInputPins() : true;
}

void UEditor_GraphNode_Template::AllocateDefaultPins()
{
	UEdGraphNode::AllocateDefaultPins();
	if (HasInputPins())
		CreatePin(EGPD_Input, DefualtPinType, InPinName);
	if (HasOutputPins())
		CreatePin(EGPD_Output, DefualtPinType, OutPinName);
}

FText UEditor_GraphNode_Template::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (BP_Node_Template)
	{
		return FText::FromString(BP_Node_Template->GetName());
	}
	return FText::GetEmpty();
}

void UEditor_GraphNode_Template::PrepareForCopying()
{
	if (BP_Node_Template)
		BP_Node_Template->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
	UEdGraphNode::PrepareForCopying();
}

void UEditor_GraphNode_Template::DestroyNode()
{
	if (BP_Node_Template)
	{
		BP_Node_Template->GetGraph()->RemoveNode(BP_Node_Template);
		BP_Node_Template->ConditionalBeginDestroy();
		BP_Node_Template = nullptr;
	}
	UEdGraphNode::DestroyNode();
}

void UEditor_GraphNode_Template::AutowireNewNode(UEdGraphPin * FromPin)
{
	if (FromPin)
	{
		UEdGraphNode::AutowireNewNode(FromPin);
		if (FromPin->Direction == EEdGraphPinDirection::EGPD_Input)
		{
			if (GetSchema()->TryCreateConnection(FromPin, FindPin(OutPinName)))
			{
				FromPin->GetOwningNode()->NodeConnectionListChanged();
			}
		}
		if (FromPin->Direction == EEdGraphPinDirection::EGPD_Output)
		{
			if (GetSchema()->TryCreateConnection(FromPin, FindPin(InPinName)))
			{
				FromPin->GetOwningNode()->NodeConnectionListChanged();
			}
		}
	}
}


void UEditor_GraphNode_Template::GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{

	if (!Context->bIsDebugging)
	{
		// add an option to convert to single frame
		{
			FToolMenuSection& Section = Menu->AddSection(NAME_None, LOCTEXT("NodeActionsMenuHeader", "节点操作"));
			Section.AddMenuEntry(FGenericCommands::Get().Delete);
			Section.AddMenuEntry(FGenericCommands::Get().Cut);
			Section.AddMenuEntry(FGenericCommands::Get().Copy);
			Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
			Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
		}
	}
}

void UEditor_GraphNode_Template::SetAssetNode(UBP_GraphNode_Template * InNode)
{
	BP_Node_Template = InNode;
}

UBP_GraphNode_Template* UEditor_GraphNode_Template::GetAssetNode()
{
	return BP_Node_Template;
}

void UEditor_GraphNode_Template::PostCopyNode()
{
	if (BP_Node_Template)
	{
		UEdGraph* EdGraph = GetGraph();
		UObject* ParentAsset = EdGraph ? EdGraph->GetOuter() : nullptr;
		BP_Node_Template->Rename(nullptr, ParentAsset, REN_DontCreateRedirectors | REN_DoNotDirty);
		BP_Node_Template->ClearFlags(RF_Transient);
	}
}

#undef LOCTEXT_NAMESPACE