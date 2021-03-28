// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/Toolkits/GraphEditor_Template.h"
#include <Misc/NotifyHook.h>
#include <IDetailsView.h>
#include <PropertyEditorModule.h>
#include <Modules/ModuleManager.h>
#include <Slate.h>
#include <Kismet2/BlueprintEditorUtils.h>
#include <GraphEditorActions.h>
#include <EdGraphUtilities.h>
#include <Editor.h>

#include "EditorGraph_Template/Blueprint/EditorGraph_Blueprint_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorNodes/Editor_GraphNode_Template.h"
#include "EditorGraph_Template_Editor/Utility/GraphEditor_Template_Log.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorGraph_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorGraphSchema_Template.h"
#include "EditorGraph_Template_Editor/BlueprintModes/DesignerApplicationMode_Template.h"
#include "EditorGraph_Template_Editor/BlueprintModes/GraphApplicationMode_Template.h"
#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"


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

	GetDesignerGraph()->OwningGraphEditor = nullptr;
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
		DesignerGraph_Template->EdGraph = CastChecked<UEditorGraph_Template>(FBlueprintEditorUtils::CreateNewGraph(DesignerGraph_Template.Get(), NAME_None, UEditorGraph_Template::StaticClass(), UEditorGraphSchema_Template::StaticClass()));
		DesignerGraph_Template->EdGraph->bAllowDeletion = false;

		//Give the schema a chance to fill out any required nodes (like the results node)
		const UEdGraphSchema* Schema = DesignerGraph_Template->EdGraph->GetSchema();
		Schema->CreateDefaultNodesForGraph(*DesignerGraph_Template->EdGraph);
	}

	GetDesignerGraph()->OwningGraphEditor = this;

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
	if (DesignerGraph_Template.IsValid())
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
	Super::InitalizeExtenders();
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
	FGraphAppearanceInfo AppearanceInfo = Super::GetGraphAppearance(InGraph);

	return AppearanceInfo;
}

TSubclassOf<UEdGraphSchema> FGraphEditor_Template::GetDefaultSchemaClass() const
{
	return UEditorGraphSchema_Template::StaticClass();
}

class UEditorGraph_Blueprint_Template* FGraphEditor_Template::GetTemplateBlueprintObj() const
{
	return Cast<UEditorGraph_Blueprint_Template>(GetBlueprintObj());
}

UEditorGraph_Template* FGraphEditor_Template::GetDesignerGraph() const
{
	return Cast<UEditorGraph_Template>(DesignerGraph_Template->EdGraph);
}

void FGraphEditor_Template::SaveAsset_Execute()
{
	if (DesignerGraph_Template.IsValid())
	{
		if (UEditorGraph_Template* EdGraph = Cast<UEditorGraph_Template>(DesignerGraph_Template->EdGraph))
		{
			EdGraph->BuildGraph();
		}
	}
	Super::SaveAsset_Execute();
}

void FGraphEditor_Template::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	Super::RegisterTabSpawners(InTabManager);
}

void FGraphEditor_Template::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	Super::UnregisterTabSpawners(InTabManager);
}

// END Commands and binding
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE