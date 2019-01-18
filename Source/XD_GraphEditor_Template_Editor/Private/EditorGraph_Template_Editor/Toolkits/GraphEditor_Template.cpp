// Fill out your copyright notice in the Description page of Project Settings.
#include "GraphEditor_Template.h"
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

FGraphEditor_Template::FGraphEditor_Template()
{

}

FGraphEditor_Template::~FGraphEditor_Template()
{
	UEditorGraph_Blueprint_Template* EditorGraph_Blueprint = GetTemplateBlueprintObj();
	if (EditorGraph_Blueprint)
	{
		EditorGraph_Blueprint->OnCompiled().RemoveAll(this);
	}
}

FLinearColor FGraphEditor_Template::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FName FGraphEditor_Template::GetToolkitFName() const
{
	return FName("Graph Editor Template");
}

FText FGraphEditor_Template::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Graph Editor Template");
}

FString FGraphEditor_Template::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Graph").ToString();
}

void FGraphEditor_Template::InitGarph_TemplateEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UEditorGraph_Blueprint_Template* InBP)
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
		EditorGraph_Blueprint->OnCompiled().AddRaw(this, &FGraphEditor_Template::BlueprintCompiled);
	}
}

void FGraphEditor_Template::BlueprintCompiled(class UBlueprint* Blueprint)
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

void FGraphEditor_Template::InitalizeExtenders()
{
	FBlueprintEditor::InitalizeExtenders();
}

void FGraphEditor_Template::RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated /*= false*/)
{
	TSharedPtr<FGraphEditor_Template> ThisPtr(SharedThis(this));

	// Create the modes and activate one (which will populate with a real layout)
	TArray<TSharedRef<FApplicationMode>> TempModeList;

	TSharedRef<FDesignerApplicationMode_Template> DesignerApplicationMode = MakeShareable(new FDesignerApplicationMode_Template(ThisPtr));
	TSharedRef<FGraphApplicationMode_Template> FGraphApplicationMode = MakeShareable(new FGraphApplicationMode_Template(ThisPtr));

 	AddApplicationMode(DesignerApplicationMode->GetModeName(), DesignerApplicationMode);
 	AddApplicationMode(FGraphApplicationMode->GetModeName(), FGraphApplicationMode);
 
 	SetCurrentMode(DesignerApplicationMode->GetModeName());
}

FGraphAppearanceInfo FGraphEditor_Template::GetGraphAppearance(class UEdGraph* InGraph) const
{
	FGraphAppearanceInfo AppearanceInfo = FBlueprintEditor::GetGraphAppearance(InGraph);

	return AppearanceInfo;
}

void FGraphEditor_Template::AppendExtraCompilerResults(TSharedPtr<class IMessageLogListing> ResultsListing)
{
	FBlueprintEditor::AppendExtraCompilerResults(ResultsListing);
}

TSubclassOf<UEdGraphSchema> FGraphEditor_Template::GetDefaultSchemaClass() const
{
	return UEditorGraphSchema_Template::StaticClass();
}

class UEditorGraph_Blueprint_Template* FGraphEditor_Template::GetTemplateBlueprintObj() const
{
	return Cast<UEditorGraph_Blueprint_Template>(GetBlueprintObj());
}

UEditorGraph_Template* FGraphEditor_Template::GetEditorGraph() const
{
	return Cast<UEditorGraph_Template>(DesignerGraph_Template->EdGraph);
}

void FGraphEditor_Template::SaveAsset_Execute()
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

void FGraphEditor_Template::RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
{
	FBlueprintEditor::RegisterTabSpawners(TabManager);
}

void FGraphEditor_Template::UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
{
	FBlueprintEditor::UnregisterTabSpawners(TabManager);
}

// END Commands and binding
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE