// Fill out your copyright notice in the Description page of Project Settings.
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
#include "Editor_GraphNode_Template.h"
#include "Editor.h"
#include "DesignerApplicationMode_Template.h"
#include "GraphApplicationMode_Template.h"


#define LOCTEXT_NAMESPACE "GraphEditorToolkit_Template"

FGraphEditorToolkit_Template::FGraphEditorToolkit_Template()
{

}

FGraphEditorToolkit_Template::~FGraphEditorToolkit_Template()
{
	UEditorGraph_Blueprint_Template* EditorGraph_Blueprint = GetTemplateBlueprintObj();
	if (EditorGraph_Blueprint)
	{
		EditorGraph_Blueprint->OnCompiled().RemoveAll(this);
	}
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
	InitBlueprintEditor(InMode, InToolkitHost, { InBP }, true);

	UpdatePreviewActor(GetBlueprintObj(), true);

	UEditorGraph_Blueprint_Template* EditorGraph_Blueprint = GetTemplateBlueprintObj();
	if (EditorGraph_Blueprint)
	{
		EditorGraph_Blueprint->OnCompiled().AddRaw(this, &FGraphEditorToolkit_Template::BlueprintCompiled);
	}
}

void FGraphEditorToolkit_Template::BlueprintCompiled(class UBlueprint* Blueprint)
{
	if (DesignerGraph_Template)
	{
		if (UEditorGraph_Template* EdGraph = Cast<UEditorGraph_Template>(DesignerGraph_Template->EdGraph))
		{
			EdGraph->BuildGraph();
			EdGraph->RefreshNodes();
		}
	}
}

void FGraphEditorToolkit_Template::InitalizeExtenders()
{
	FBlueprintEditor::InitalizeExtenders();
}

void FGraphEditorToolkit_Template::RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated /*= false*/)
{
	TSharedPtr<FGraphEditorToolkit_Template> ThisPtr(SharedThis(this));

	// Create the modes and activate one (which will populate with a real layout)
	TArray<TSharedRef<FApplicationMode>> TempModeList;

	TSharedRef<FDesignerApplicationMode_Template> DesignerApplicationMode = MakeShareable(new FDesignerApplicationMode_Template(ThisPtr));
	TSharedRef<FGraphApplicationMode_Template> FGraphApplicationMode = MakeShareable(new FGraphApplicationMode_Template(ThisPtr));

 	AddApplicationMode(DesignerApplicationMode->GetModeName(), DesignerApplicationMode);
 	AddApplicationMode(FGraphApplicationMode->GetModeName(), FGraphApplicationMode);
 
 	SetCurrentMode(DesignerApplicationMode->GetModeName());
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

UEditorGraph_Template* FGraphEditorToolkit_Template::GetEditorGraph() const
{
	return Cast<UEditorGraph_Template>(DesignerGraph_Template->EdGraph);
}

void FGraphEditorToolkit_Template::SaveAsset_Execute()
{
	if (DesignerGraph_Template)
	{
		if (UEditorGraph_Template* EdGraph = Cast<UEditorGraph_Template>(DesignerGraph_Template->EdGraph))
		{
			EdGraph->BuildGraph();
		}
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

// END Commands and binding
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE