// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/EditorGraph/EditorNodes/Editor_GraphNode_Template.h"
#include <IDetailsView.h>
#include <PropertyEditorModule.h>
#include <Modules/ModuleManager.h>
#include <Framework/Commands/GenericCommands.h>
#include <ToolMenu.h>
#include <GraphEditorActions.h>

#include "EditorGraph_Template/Nodes/BP_GraphNode_Template.h"
#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorGraph_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/SlateWidgets/SGraphNode_Template.h"
#include "EditorGraph_Template_Editor/Utility/GraphEditor_Template_Log.h"

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
	View->SetObject(AssetNode);
	return View;
}

void UEditor_GraphNode_Template::UpdateVisualNode()
{
	SlateNode->UpdateGraphNode();
}

void UEditor_GraphNode_Template::SaveNodesAsChildren(TArray<UEdGraphNode*>& Children)
{
    for (UEdGraphNode* Child : Children)
    {
        UEditor_GraphNode_Template* Node = Cast<UEditor_GraphNode_Template>(Child);
        if (Node)
            AssetNode->LinkArgumentNodeAsChild(Node->AssetNode);
    }
}

bool UEditor_GraphNode_Template::HasOutputPins()
{
	return AssetNode ? AssetNode->HasOutputPins() : true;
}

bool UEditor_GraphNode_Template::HasInputPins()
{
	return AssetNode ? AssetNode->HasInputPins() : true;
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
    switch (TitleType)
    {
    case ENodeTitleType::Type::MenuTitle:
        return AssetNode->GetNodeTitle();
    default:
        FText Title = GetEdNodeName();
        return Title.IsEmpty() ? AssetNode->GetNodeTitle() : Title;
    }
}

void UEditor_GraphNode_Template::PrepareForCopying()
{
	if (AssetNode)
		AssetNode->Rename(nullptr, this, REN_DontCreateRedirectors | REN_DoNotDirty);
	UEdGraphNode::PrepareForCopying();
}

void UEditor_GraphNode_Template::DestroyNode()
{
	if (AssetNode)
	{
		AssetNode->GetGraph()->RemoveNode(AssetNode);
		AssetNode->ConditionalBeginDestroy();
		AssetNode = nullptr;
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
	FToolMenuSection& Section = Menu->AddSection(TEXT("NodeActionsMenu"), LOCTEXT("NodeActionsMenuHeader", "节点操作"));
	Section.AddMenuEntry(FGenericCommands::Get().Delete);
	Section.AddMenuEntry(FGenericCommands::Get().Cut);
	Section.AddMenuEntry(FGenericCommands::Get().Copy);
	Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
	Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
}

void UEditor_GraphNode_Template::SetAssetNode(UBP_GraphNode_Template * InNode)
{
	AssetNode = InNode;
}

UBP_GraphNode_Template* UEditor_GraphNode_Template::GetAssetNode()
{
	return AssetNode;
}

void UEditor_GraphNode_Template::PostCopyNode()
{
	if (AssetNode)
	{
		UEdGraph* EdGraph = GetGraph();
		UObject* ParentAsset = EdGraph ? EdGraph->GetOuter() : nullptr;
		AssetNode->Rename(nullptr, ParentAsset, REN_DontCreateRedirectors | REN_DoNotDirty);
		AssetNode->ClearFlags(RF_Transient);
	}
}

bool UEditor_GraphNode_Template::RenameUniqueNode(const FText & NewName)
{
	bool bRenamedNode = false;

	UEditorGraph_Template* EdGraph = Cast<UEditorGraph_Template>(GetGraph());

	if (EdGraph->IsNameUnique(NewName))
	{
		Modify();
		SetEdNodeName(NewName);
		bRenamedNode = true;
	}
	return bRenamedNode;
}
FText UEditor_GraphNode_Template::GetEdNodeName() const
{
	return EdNodeName;
}
void UEditor_GraphNode_Template::SetEdNodeName(const FText & Name)
{
	EdNodeName = Name;
}

void UEditor_GraphNode_Template::SetEdNodeName(const FName & Name)
{
	SetEdNodeName(FText::FromName(Name));
}


#undef LOCTEXT_NAMESPACE