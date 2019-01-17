// Fill out your copyright notice in the Description page of Project Settings.
#include "Editor_GraphNode_Template.h"
#include "BP_GraphNode_Template.h"
#include "BP_Graph_Template.h"
#include "EditorGraph_Template.h"
#include "SGraphNode_Template.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "GraphEditor_Template_Log.h"
#include "GenericCommands.h"
#include "GraphEditorActions.h"

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

TSharedPtr<SWidget> UEditor_GraphNode_Template::GetContentWidget()
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	TSharedPtr<IDetailsView> View = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	View->SetObject(BP_Node_Template);
	return View;
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


void UEditor_GraphNode_Template::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
{
	FMenuBuilder* MenuBuilder = Context.MenuBuilder;
	MenuBuilder->BeginSection(NAME_None, LOCTEXT("NodeActionsMenuHeader", "节点操作"));
	{
		MenuBuilder->AddMenuEntry(FGenericCommands::Get().Delete);
		MenuBuilder->AddMenuEntry(FGenericCommands::Get().Cut);
		MenuBuilder->AddMenuEntry(FGenericCommands::Get().Copy);
		MenuBuilder->AddMenuEntry(FGenericCommands::Get().Duplicate);

		MenuBuilder->AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
	}
	MenuBuilder->EndSection();
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