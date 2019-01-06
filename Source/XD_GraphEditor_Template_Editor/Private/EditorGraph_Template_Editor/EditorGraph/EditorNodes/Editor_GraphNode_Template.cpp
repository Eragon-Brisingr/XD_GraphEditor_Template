// Copyright (c) 2018 Jhonny Hueller
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