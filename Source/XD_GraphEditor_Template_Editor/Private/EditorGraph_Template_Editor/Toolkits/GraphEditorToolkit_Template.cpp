// Copyright (c) 2018 Jhonny Hueller
#include "EditorGraph_Template_Editor/Toolkits/GraphEditorToolkit_Template.h"
#include <PropertyEditorModule.h>
#include <Slate.h>
#include <GraphEditorActions.h>
#include <EdGraphUtilities.h>
#include <Editor.h>
#include <HAL/PlatformApplicationMisc.h>
#include <Kismet2/BlueprintEditorUtils.h>

#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorGraphSchema_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorGraph_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorNodes/Editor_GraphNode_Template.h"
#include "EditorGraph_Template_Editor/Utility/GraphEditor_Template_Log.h"


#define LOCTEXT_NAMESPACE "GraphEditorToolkit_Template"

const FName FGraphEditorToolkit_Template::DetailsTabId(TEXT("GraphEditorToolkit_TemplateDetailsTabId"));
const FName FGraphEditorToolkit_Template::GraphTabId(TEXT("GraphEditorToolkit_TemplateGraphTabId"));


FGraphEditorToolkit_Template::FGraphEditorToolkit_Template()
{
	GEditor->OnBlueprintCompiled().AddRaw(this,&FGraphEditorToolkit_Template::BlueprintCompiled);
}

FGraphEditorToolkit_Template::~FGraphEditorToolkit_Template()
{
	GEditor->OnBlueprintCompiled().RemoveAll(this);
}

FGraphPanelSelectionSet FGraphEditorToolkit_Template::GetSelectedNodes()
{
	return EdGraphEditor->GetSelectedNodes();
}

TSharedRef<SDockTab> FGraphEditorToolkit_Template::HandleTabManagerSpawnTabDetails(const FSpawnTabArgs & Args)
{

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bCustomNameAreaLocation = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	DetailsWidget = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsWidget->SetObject(GraphAsset);
	
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			DetailsWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FGraphEditorToolkit_Template::HandleTabManagerSpawnTabGraph(const FSpawnTabArgs & Args)
{

	if (GraphAsset->EdGraph != nullptr)
	{
		EdGraphEditor = SNew(SGraphEditor)
			.AdditionalCommands(GraphEditorCommands)
			.GraphToEdit(GraphAsset->EdGraph);
	}
	else
	{
		GraphEditor_Template_Error_Log("There is no editor graph inside the graph asset.");
	}
	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			EdGraphEditor.ToSharedRef()
		];
}

FLinearColor FGraphEditorToolkit_Template::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FName FGraphEditorToolkit_Template::GetToolkitFName() const
{
	return FName("Graph Editor");
}

FText FGraphEditorToolkit_Template::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Graph Editor");
}

FString FGraphEditorToolkit_Template::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Graph").ToString();
}

void FGraphEditorToolkit_Template::InitGraphAssetEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UBP_Graph_Template * InGraph)
{
	GraphAsset = InGraph;
	if (GraphAsset->EdGraph == nullptr)
	{
		GraphEditor_Template_Log("Creating a new graph.");
		GraphAsset->EdGraph = CastChecked<UEditorGraph_Template>(FBlueprintEditorUtils::CreateNewGraph(GraphAsset, NAME_None, UEditorGraph_Template::StaticClass(), UEditorGraphSchema_Template::StaticClass()));
		GraphAsset->EdGraph->bAllowDeletion = false;

		//Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = GraphAsset->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*GraphAsset->EdGraph);
	}

	FGenericCommands::Register();
	FGraphEditorCommands::Register();
	BindToolkitCommands();

	TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("LayoutName")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.2f)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.8f)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(DetailsTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.15f)
				)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(GraphTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.85f)
				)
			)
		);

	FAssetEditorToolkit::InitAssetEditor(InMode, InToolkitHost, FName("GraphEditorIdentifier"), Layout, true, true, GraphAsset);
}

void FGraphEditorToolkit_Template::BlueprintCompiled()
{
	UEdGraph* EdGraph = EdGraphEditor->GetCurrentGraph();
	if (UEditorGraph_Template* MyGraph = Cast<UEditorGraph_Template>(EdGraph))
		MyGraph->RefreshNodes();
}

void FGraphEditorToolkit_Template::SaveAsset_Execute()
{
	if (GraphAsset && GraphAsset->EdGraph)
	{
		UEditorGraph_Template* EdGraph = Cast<UEditorGraph_Template>(GraphAsset->EdGraph);
		EdGraph->SaveGraph();
	}
	FAssetEditorToolkit::SaveAsset_Execute();
}

void FGraphEditorToolkit_Template::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("EditorGraph_TemplateToolkitWorkspaceMenu", "Graph Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();
	
	TabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FGraphEditorToolkit_Template::HandleTabManagerSpawnTabDetails))
		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef);
	TabManager->RegisterTabSpawner(GraphTabId, FOnSpawnTab::CreateSP(this, &FGraphEditorToolkit_Template::HandleTabManagerSpawnTabGraph))
		.SetDisplayName(LOCTEXT("GraphTab", "Graph Editor"))
		.SetGroup(WorkspaceMenuCategoryRef);

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FGraphEditorToolkit_Template::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(GraphTabId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands and Bindings
void FGraphEditorToolkit_Template::BindToolkitCommands()
{
	if (!GraphEditorCommands.IsValid())
	{
		GraphEditorCommands = MakeShareable(new FUICommandList());

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnCommandSelectAllNodes),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanSelectAllNodes)
		);

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Cut,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnCommandCut),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanCutNodes)
		);

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Copy,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnCommandCopy),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanCopyNodes)
		);

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Paste,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnCommandPaste),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanPasteNodes)
		);

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnCommandDuplicate),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanDuplicateNodes)
		);

		GraphEditorCommands->MapAction
		(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnCommandDelete),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanDeleteNodes)
		);

	}
}

void FGraphEditorToolkit_Template::OnCommandSelectAllNodes()
{
	if (EdGraphEditor.IsValid())
		EdGraphEditor->SelectAllNodes();
}

bool FGraphEditorToolkit_Template::CanSelectAllNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnCommandCut()
{
	OnCommandCopy();

	const FGraphPanelSelectionSet OldSelectedNodes = EdGraphEditor->GetSelectedNodes();
	EdGraphEditor->ClearSelectionSet();
	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
		if (Node && Node->CanDuplicateNode())
		{
			EdGraphEditor->SetNodeSelection(Node, true);
		}
	}

	OnCommandDelete();

	EdGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
		if (Node)
			EdGraphEditor->SetNodeSelection(Node, true);
	}
}

bool FGraphEditorToolkit_Template::CanCutNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnCommandCopy()
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

bool FGraphEditorToolkit_Template::CanCopyNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnCommandPaste()
{

	const FVector2D PasteLocation = EdGraphEditor->GetPasteLocation();

	UEdGraph* EdGraph = EdGraphEditor->GetCurrentGraph();
	EdGraph->Modify();
	EdGraphEditor->ClearSelectionSet();

	FString ExportedText;
	FPlatformApplicationMisc::ClipboardPaste(ExportedText);
	TSet<UEdGraphNode*> ImportedNodes;
	FEdGraphUtilities::ImportNodesFromText(EdGraph, ExportedText, ImportedNodes);

	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
	{
		UEditor_GraphNode_Template* Node = Cast<UEditor_GraphNode_Template>(*It);
		GraphAsset->AddNode(Node->AssetNode);
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
		EdGraphEditor->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + PasteLocation.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + PasteLocation.Y;

		Node->SnapToGrid(16);

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}

	EdGraphEditor->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}

}

bool FGraphEditorToolkit_Template::CanPasteNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnCommandDuplicate()
{
	OnCommandCopy();
	OnCommandPaste();
}

bool FGraphEditorToolkit_Template::CanDuplicateNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnCommandDelete()
{

	EdGraphEditor->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	EdGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(SelectedNodes); It; ++It)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*It))
		{
			Node->Modify();
			Node->DestroyNode();
		}
	}
}

bool FGraphEditorToolkit_Template::CanDeleteNodes()
{
	return true;
}

// END Commands and binding
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE