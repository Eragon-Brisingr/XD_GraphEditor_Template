// Copyright (c) 2018 Jhonny Hueller
#include "GraphEditorToolkit_Template.h"
#include "GraphEditor_Template_Log.h"
#include "PropertyEditorModule.h"
#include "ModuleManager.h"
#include "Slate.h"
#include "BlueprintEditorUtils.h"
#include "GraphEditorActions.h"
#include "EditorGraph_Template.h"
#include "EditorGraphSchema_Template.h"
#include "EdGraphUtilities.h"
#include "PlatformApplicationMisc.h"
#include "Editor_GraphNode_Template.h"
#include "Editor.h"
#include "DesignerApplicationMode_Template.h"
#include "GraphApplicationMode_Template.h"


#define LOCTEXT_NAMESPACE "GraphEditorToolkit_Template"

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
	DetailsWidget->SetObject(DesignerGraph_Template);

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		[
			DetailsWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FGraphEditorToolkit_Template::HandleTabManagerSpawnTabGraph(const FSpawnTabArgs & Args)
{

	if (DesignerGraph_Template->EdGraph != nullptr)
	{
		EdGraphEditor = SNew(SGraphEditor)
			.AdditionalCommands(DesignerEditorCommands)
			.GraphToEdit(DesignerGraph_Template->EdGraph);
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
	return FName("Graph Editor Template");
}

FText FGraphEditorToolkit_Template::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Graph Editor Template");
}

FString FGraphEditorToolkit_Template::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Graph").ToString();
}

void FGraphEditorToolkit_Template::InitGarph_TemplateEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UEditorGraph_Blueprint_Template* InBP)
{
	DesignerGraph_Template = InBP->DesignerGraph_Template;

	if (DesignerGraph_Template->EdGraph == nullptr)
	{
		GraphEditor_Template_Log("Creating a new graph.");
		DesignerGraph_Template->EdGraph = CastChecked<UEditorGraph_Template>(FBlueprintEditorUtils::CreateNewGraph(DesignerGraph_Template, NAME_None, UEditorGraph_Template::StaticClass(), UEditorGraphSchema_Template::StaticClass()));
		DesignerGraph_Template->EdGraph->bAllowDeletion = false;

		//Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = DesignerGraph_Template->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*DesignerGraph_Template->EdGraph);
	}

// 	FGenericCommands::Register();
// 	FGraphEditorCommands::Register();
	BindToolkitCommands();

	InitBlueprintEditor(InMode, InToolkitHost, { InBP }, true);

	UpdatePreviewActor(GetBlueprintObj(), true);
}

void FGraphEditorToolkit_Template::BlueprintCompiled()
{
// 	UEdGraph* EdGraph = EdGraphEditor->GetCurrentGraph();
// 	if (UEditorGraph_Template* MyGraph = Cast<UEditorGraph_Template>(EdGraph))
// 		MyGraph->RefreshNodes();
}

void FGraphEditorToolkit_Template::InitalizeExtenders()
{
	FBlueprintEditor::InitalizeExtenders();

// 	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);
// 
// 	// Extend the File menu with asset actions
// 	MenuExtender->AddMenuExtension(
// 		"FileLoadAndSave",
// 		EExtensionHook::After,
// 		GetToolkitCommands(),
// 		FMenuExtensionDelegate::CreateSP(this, &FWidgetBlueprintEditor::FillFileMenu));
// 
// 	AddMenuExtender(MenuExtender);
}

void FGraphEditorToolkit_Template::RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated /*= false*/)
{
	//FBlueprintEditor::RegisterApplicationModes(InBlueprints, bShouldOpenInDefaultsMode, bNewlyCreated);

	TSharedPtr<FGraphEditorToolkit_Template> ThisPtr(SharedThis(this));

	// Create the modes and activate one (which will populate with a real layout)
	TArray<TSharedRef<FApplicationMode>> TempModeList;
	TempModeList.Add(MakeShareable(new FDesignerApplicationMode_Template(ThisPtr)));
	TempModeList.Add(MakeShareable(new FGraphApplicationMode_Template(ThisPtr)));

	for (TSharedRef<FApplicationMode>& AppMode : TempModeList)
	{
		AddApplicationMode(AppMode->GetModeName(), AppMode);
	}

	SetCurrentMode(FBlueprintApplicationModesTemplate::DesignerMode);
}

FGraphAppearanceInfo FGraphEditorToolkit_Template::GetGraphAppearance(class UEdGraph* InGraph) const
{
	FGraphAppearanceInfo AppearanceInfo = FBlueprintEditor::GetGraphAppearance(InGraph);

	return AppearanceInfo;
}

void FGraphEditorToolkit_Template::AppendExtraCompilerResults(TSharedPtr<class IMessageLogListing> ResultsListing)
{
	FBlueprintEditor::AppendExtraCompilerResults(ResultsListing);
}

TSubclassOf<UEdGraphSchema> FGraphEditorToolkit_Template::GetDefaultSchemaClass() const
{
	return UEditorGraphSchema_Template::StaticClass();
}

class UEditorGraph_Blueprint_Template* FGraphEditorToolkit_Template::GetTemplateBlueprintObj() const
{
	return Cast<UEditorGraph_Blueprint_Template>(GetBlueprintObj());
}

void FGraphEditorToolkit_Template::SaveAsset_Execute()
{
	if (DesignerGraph_Template && DesignerGraph_Template->EdGraph)
	{
		UEditorGraph_Template* EdGraph = Cast<UEditorGraph_Template>(DesignerGraph_Template->EdGraph);
		EdGraph->SaveGraph();
	}
	FBlueprintEditor::SaveAsset_Execute();
}

void FGraphEditorToolkit_Template::RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
{
	FBlueprintEditor::RegisterTabSpawners(TabManager);
}

void FGraphEditorToolkit_Template::UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
{
	FBlueprintEditor::UnregisterTabSpawners(TabManager);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Commands and Bindings
void FGraphEditorToolkit_Template::BindToolkitCommands()
{
	if (!DesignerEditorCommands.IsValid())
	{
		DesignerEditorCommands = MakeShareable(new FUICommandList());

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnDesignerCommandSelectAllNodes),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanDesignerSelectAllNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Cut,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnDesignerCommandCut),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanDesignerCutNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Copy,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnDesignerCommandCopy),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanDesignerCopyNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Paste,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnDesignerCommandPaste),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanDesignerPasteNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnDesignerCommandDuplicate),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanDesignerDuplicateNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::OnDesignerCommandDelete),
			FCanExecuteAction::CreateRaw(this, &FGraphEditorToolkit_Template::CanDesignerDeleteNodes)
		);

	}
}

void FGraphEditorToolkit_Template::OnDesignerCommandSelectAllNodes()
{
	if (EdGraphEditor.IsValid())
		EdGraphEditor->SelectAllNodes();
}

bool FGraphEditorToolkit_Template::CanDesignerSelectAllNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnDesignerCommandCut()
{
	OnDesignerCommandCopy();

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

	OnDesignerCommandDelete();

	EdGraphEditor->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
		if (Node)
			EdGraphEditor->SetNodeSelection(Node, true);
	}
}

bool FGraphEditorToolkit_Template::CanDesignerCutNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnDesignerCommandCopy()
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

bool FGraphEditorToolkit_Template::CanDesignerCopyNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnDesignerCommandPaste()
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
		DesignerGraph_Template->AddNode(Node->AssetNode);
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

bool FGraphEditorToolkit_Template::CanDesignerPasteNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnDesignerCommandDuplicate()
{
	OnDesignerCommandCopy();
	OnDesignerCommandPaste();
}

bool FGraphEditorToolkit_Template::CanDesignerDuplicateNodes()
{
	return true;
}

void FGraphEditorToolkit_Template::OnDesignerCommandDelete()
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

bool FGraphEditorToolkit_Template::CanDesignerDeleteNodes()
{
	return true;
}

// END Commands and binding
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE