// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorGraph_Template_Editor/BlueprintModes/DesignerApplicationMode_Template.h"
#include <WorkflowOrientedApp/WorkflowTabFactory.h>
#include <BlueprintEditor.h>
#include <PropertyEditorModule.h>
#include <GraphEditor.h>
#include <Framework/Commands/GenericCommands.h>
#include <EdGraphUtilities.h>
#include <HAL/PlatformApplicationMisc.h>
#include <SBlueprintEditorToolbar.h>
#include <Kismet2/BlueprintEditorUtils.h>
#include <PropertyEditorDelegates.h>
#include <SKismetInspector.h>
#include <IDetailCustomization.h>
#include <DetailCategoryBuilder.h>
#include <DetailLayoutBuilder.h>
#include <Widgets/Input/SComboButton.h>
#include <DetailWidgetRow.h>
#include <ObjectEditorUtils.h>
#include <Kismet2/KismetEditorUtilities.h>
#include <Widgets/Layout/SWidgetSwitcher.h>
#include <K2Node_ComponentBoundEvent.h>
#include <IDetailPropertyExtensionHandler.h>
#include <Framework/MultiBox/MultiBoxBuilder.h>
#include <Framework/Application/SlateApplication.h>
#include <ScopedTransaction.h>

#include "EditorGraph_Template/Nodes/BP_GraphNode_Template.h"
#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"
#include "EditorGraph_Template_Editor/Toolkits/GraphEditor_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorNodes/Editor_GraphNode_Template.h"
#include "EditorGraph_Template/Blueprint/EditorGraph_Blueprint_Template.h"
#include "EditorGraph_Template_Editor/SCompoundWidget/SPropertyBinding_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorGraph_Template.h"
#include "EditorGraph_Template_Editor/SCompoundWidget/SDetailsView_Template.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

const FName FDesignerApplicationMode_Template::DetailsTabId(TEXT("Designer_Template_DetailsTabId"));
const FName FDesignerApplicationMode_Template::GraphTabId(TEXT("Designer_Template_GraphTabId"));

struct FFunctionInfo
{
	FFunctionInfo()
		: Function(nullptr)
	{
	}

	FText DisplayName;
	FString Tooltip;

	FName FuncName;
	UFunction* Function;
};

struct FDesignerDetailsSummoner_Template : public FWorkflowTabFactory
{
public:
	FDesignerDetailsSummoner_Template(class FDesignerApplicationMode_Template* DesignerApplicationMode, TSharedPtr<class FGraphEditor_Template> InDesignGraphEditor);

	FDesignerApplicationMode_Template* DesignerApplicationMode;
	TWeakPtr<class FGraphEditor_Template> InDesignGraphEditor;

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FDesignerDetailsSummoner_Template::FDesignerDetailsSummoner_Template(class FDesignerApplicationMode_Template* DesignerApplicationMode, TSharedPtr<class FGraphEditor_Template> InDesignGraphEditor)
	: FWorkflowTabFactory(FDesignerApplicationMode_Template::DetailsTabId, InDesignGraphEditor),
	DesignerApplicationMode(DesignerApplicationMode),
	InDesignGraphEditor(InDesignGraphEditor)
{
	TabLabel = LOCTEXT("DesingerDetails_Template_TabLabel", "细节");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DesingerDetails_Template_ViewMenu_Desc", "细节");
	ViewMenuTooltip = LOCTEXT("DesingerDetails_Template_ViewMenu_ToolTip", "Show the Details");
}

TSharedRef<SWidget> FDesignerDetailsSummoner_Template::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedRef<SDetailsView_Template> DesignerDetails = SNew(SDetailsView_Template, InDesignGraphEditor);
	DesignerApplicationMode->DesignerDetails = DesignerDetails;
	return DesignerDetails;
}

struct FDesignerGraphSummoner_Template : public FWorkflowTabFactory
{
public:
	FDesignerGraphSummoner_Template(class FDesignerApplicationMode_Template* DesignerApplicationMode, TSharedPtr<class FGraphEditor_Template> InDesignGraphEditor);

	FDesignerApplicationMode_Template* DesignerApplicationMode;
	TWeakPtr<class FGraphEditor_Template> InDesignGraphEditor;

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FDesignerGraphSummoner_Template::FDesignerGraphSummoner_Template(class FDesignerApplicationMode_Template* DesignerApplicationMode, TSharedPtr<class FGraphEditor_Template> InDesignGraphEditor)
	:FWorkflowTabFactory(FDesignerApplicationMode_Template::GraphTabId, InDesignGraphEditor)
	, DesignerApplicationMode(DesignerApplicationMode)
	, InDesignGraphEditor(InDesignGraphEditor)
{
	TabLabel = LOCTEXT("DesingerGraph_Template_TabLabel", "图表");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "PhysicsAssetEditor.Tabs.Graph");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DesingerGraph_Template_ViewMenu_Desc", "图表");
	ViewMenuTooltip = LOCTEXT("DesingerGraph_Template_ViewMenu_ToolTip", "Show the Garph");
}

TSharedRef<SWidget> FDesignerGraphSummoner_Template::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SAssignNew(DesignerApplicationMode->DesignerGraphEditor, SGraphEditor)
		.AdditionalCommands(DesignerApplicationMode->DesignerEditorCommands)
		.GraphToEdit(InDesignGraphEditor.Pin()->DesignerGraph_Template->EdGraph)
		.GraphEvents(DesignerApplicationMode->DesignerGraphEvents);
}

FDesignerApplicationMode_Template::FDesignerApplicationMode_Template(TSharedPtr<class FGraphEditor_Template> GraphEditorToolkit)
	:FBlueprintApplicationModeTemplate(GraphEditorToolkit, FBlueprintApplicationModesTemplate::DesignerMode)
{
	WorkspaceMenuCategory = FWorkspaceItem::NewGroup(LOCTEXT("WorkspaceMenu_Designer_Template", "Designer_Template"));

	TabLayout = FTabManager::NewLayout("Designer_Template_Layout_v1_1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.2f)
				->AddTab(GraphEditorToolkit->GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.8f)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.8f)
					->AddTab(GraphTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.2f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
			)
		);

 	TabFactories.RegisterFactory(MakeShareable(new FDesignerDetailsSummoner_Template(this, GraphEditorToolkit)));
 	TabFactories.RegisterFactory(MakeShareable(new FDesignerGraphSummoner_Template(this, GraphEditorToolkit)));

	ToolbarExtender = MakeShareable(new FExtender);

	if (UToolMenu* Toolbar = GraphEditorToolkit->RegisterModeToolbarIfUnregistered(GetModeName()))
	{
		GraphEditorToolkit->GetToolbarBuilder()->AddCompileToolbar(Toolbar);
		GraphEditorToolkit->GetToolbarBuilder()->AddDebuggingToolbar(Toolbar);
	}
	AddModeSwitchToolBarExtension();

	BindDesignerToolkitCommands();
	DesignerGraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateRaw(this, &FDesignerApplicationMode_Template::HandleSelectionChanged);
}

void FDesignerApplicationMode_Template::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	FGraphEditor_Template *BP = GetBlueprintEditor();

	BP->RegisterToolbarTab(InTabManager.ToSharedRef());
	BP->PushTabFactories(TabFactories);
}

void FDesignerApplicationMode_Template::PreDeactivateMode()
{

}

void FDesignerApplicationMode_Template::PostActivateMode()
{

}

void FDesignerApplicationMode_Template::HandleSelectionChanged(const FGraphPanelSelectionSet& SelectionSet)
{
	if (DesignerDetails.IsValid())
	{
		TSharedPtr<SDetailsView_Template> Details = DesignerDetails.Pin();

		TArray<UObject*> ShowObjects;
		for (UObject* Obj : SelectionSet)
		{
			if (UEditor_GraphNode_Template* EdGraphNode_Template = Cast<UEditor_GraphNode_Template>(Obj))
			{
				if (EdGraphNode_Template->BP_Node_Template)
				{
					ShowObjects.Add(EdGraphNode_Template->BP_Node_Template);
				}
			}
		}

		Details->PropertyView->SetObjects(ShowObjects, true);
	}
}

void FDesignerApplicationMode_Template::BindDesignerToolkitCommands()
{
	if (!DesignerEditorCommands.IsValid())
	{
		DesignerEditorCommands = MakeShareable(new FUICommandList());

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandSelectAllNodes),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerSelectAllNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Cut,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandCut),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerCutNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Copy,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandCopy),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerCopyNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Paste,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandPaste),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerPasteNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandDuplicate),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerDuplicateNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandDelete),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerDeleteNodes)
		);

	}
}

FGraphPanelSelectionSet FDesignerApplicationMode_Template::GetSelectedNodes()
{
	return GetDesignerGraphEditor()->GetSelectedNodes();
}

void FDesignerApplicationMode_Template::OnDesignerCommandSelectAllNodes()
{
	if (DesignerGraphEditor.IsValid())
	{
		GetDesignerGraphEditor()->SelectAllNodes();
	}
}

bool FDesignerApplicationMode_Template::CanDesignerSelectAllNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandCut()
{
	OnDesignerCommandCopy();

	const FGraphPanelSelectionSet OldSelectedNodes = GetDesignerGraphEditor()->GetSelectedNodes();
	GetDesignerGraphEditor()->ClearSelectionSet();
	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
		if (Node && Node->CanDuplicateNode())
		{
			GetDesignerGraphEditor()->SetNodeSelection(Node, true);
		}
	}

	OnDesignerCommandDelete();

	GetDesignerGraphEditor()->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
		if (Node)
			GetDesignerGraphEditor()->SetNodeSelection(Node, true);
	}
}

bool FDesignerApplicationMode_Template::CanDesignerCutNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandCopy()
{
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	FString ExportedText;

	for (FGraphPanelSelectionSet::TIterator it(SelectedNodes); it; ++it)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*it);
		if (Node)
			Node->PrepareForCopying();
		else
			it.RemoveCurrent();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	for (FGraphPanelSelectionSet::TIterator it(SelectedNodes); it; ++it)
	{
		UEditor_GraphNode_Template* Node = Cast<UEditor_GraphNode_Template>(*it);
		if (Node)
			Node->PostCopyNode();
	}
}

bool FDesignerApplicationMode_Template::CanDesignerCopyNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandPaste()
{
	const FVector2D PasteLocation = GetDesignerGraphEditor()->GetPasteLocation();

	UEdGraph* EdGraph = GetDesignerGraphEditor()->GetCurrentGraph();
	EdGraph->Modify();
	GetDesignerGraphEditor()->ClearSelectionSet();

	FString ExportedText;
	FPlatformApplicationMisc::ClipboardPaste(ExportedText);
	TSet<UEdGraphNode*> ImportedNodes;
	FEdGraphUtilities::ImportNodesFromText(EdGraph, ExportedText, ImportedNodes);

	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
	{
		UEditor_GraphNode_Template* Node = Cast<UEditor_GraphNode_Template>(*It);
		MyEditor_Template.Pin()->DesignerGraph_Template->AddNode(Node->BP_Node_Template);
	}

	FVector2D AvgNodePosition(0.0f, 0.0f);

	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	float InvNumNodes = 1.0f / float(ImportedNodes.Num());
	AvgNodePosition.X *= InvNumNodes;
	AvgNodePosition.Y *= InvNumNodes;

	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		GetDesignerGraphEditor()->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + PasteLocation.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + PasteLocation.Y;

		Node->SnapToGrid(16);

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}

	GetDesignerGraphEditor()->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}

}

bool FDesignerApplicationMode_Template::CanDesignerPasteNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandDuplicate()
{
	OnDesignerCommandCopy();
	OnDesignerCommandPaste();
}

bool FDesignerApplicationMode_Template::CanDesignerDuplicateNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandDelete()
{
	GetDesignerGraphEditor()->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	GetDesignerGraphEditor()->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(SelectedNodes); It; ++It)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*It))
		{
			Node->Modify();
			Node->DestroyNode();
		}
	}
}

bool FDesignerApplicationMode_Template::CanDesignerDeleteNodes()
{
	return true;
}

#undef LOCTEXT_NAMESPACE