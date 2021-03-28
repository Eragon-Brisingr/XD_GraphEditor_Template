// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "XD_GraphEditor_Template_Editor.h"
#include <Modules/ModuleManager.h>
#include <IAssetTools.h>
#include <AssetToolsModule.h>
#include <KismetCompiler.h>

#include "EditorGraph_Template/Blueprint/BlueprintGeneratedClass_Template.h"
#include "EditorGraph_Template/Blueprint/EditorGraph_Blueprint_Template.h"
#include "EditorGraph_Template_Editor/Utility/GraphEditor_ClassHelper_Template.h"
#include "EditorGraph_Template_Editor/Utility/GraphEditor_Template_Log.h"
#include "EditorGraph_Template_Editor/AssetTypeActions/GraphEditor_Template_AssetTypeActions.h"
#include "EditorGraph_Template_Editor/Compiler/BP_Compiler_Template.h"
#include "EditorGraph_Template/Nodes/BP_GraphNode_Template.h"

#define LOCTEXT_NAMESPACE "FXD_GraphEditor_Template_EditorModule"

void FXD_GraphEditor_Template_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedRef<IAssetTypeActions> Actions = MakeShareable(new FGraphEditor_Template_AssetTypeActions());
	AssetTools.RegisterAssetTypeActions(Actions);

	FKismetCompilerContext::RegisterCompilerForBP(UEditorGraph_Blueprint_Template::StaticClass(), &FXD_GraphEditor_Template_EditorModule::GetCompilerForBP);
}

void FXD_GraphEditor_Template_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	NodeHelper.Reset();
}

void FXD_GraphEditor_Template_EditorModule::StartNodeHelper()
{
	if (!NodeHelper.IsValid())
	{
		NodeHelper = MakeShareable(new FGraphEditor_ClassHelper_Template(UBP_GraphNode_Template::StaticClass()));
		FGraphEditor_ClassHelper_Template::AddObservedBlueprintClasses(UBP_GraphNode_Template::StaticClass());
	}
}

TSharedPtr<FKismetCompilerContext> FXD_GraphEditor_Template_EditorModule::GetCompilerForBP(UBlueprint* BP, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompileOptions)
{
	return TSharedPtr<FKismetCompilerContext>(new FBP_Compiler_Template(CastChecked<UEditorGraph_Blueprint_Template>(BP), InMessageLog, InCompileOptions));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FXD_GraphEditor_Template_EditorModule, XD_GraphEditor_Template_Editor)