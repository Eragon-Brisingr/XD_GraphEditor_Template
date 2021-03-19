// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/AssetTypeActions/GraphEditor_Template_AssetTypeActions.h"
#include <Modules/ModuleManager.h>

#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"
#include "EditorGraph_Template_Editor/Toolkits/GraphEditorToolkit_Template.h"
#include "XD_GraphEditor_Template_Editor.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template_AssetTypeActions"

FGraphEditor_Template_AssetTypeActions::FGraphEditor_Template_AssetTypeActions()
{
}

FText FGraphEditor_Template_AssetTypeActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "GraphTypeActions", "XD_GraphEditor_Template");
}

UClass * FGraphEditor_Template_AssetTypeActions::GetSupportedClass() const
{
	return UBP_Graph_Template::StaticClass();
}

FColor FGraphEditor_Template_AssetTypeActions::GetTypeColor() const
{
	return FColor::Red;
}

uint32 FGraphEditor_Template_AssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

bool FGraphEditor_Template_AssetTypeActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return false;
}

void FGraphEditor_Template_AssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	FXD_GraphEditor_Template_EditorModule& Helper= FModuleManager::LoadModuleChecked<FXD_GraphEditor_Template_EditorModule>("XD_GraphEditor_Template_Editor");
	Helper.StartNodeHelper();

	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto Object = InObjects.CreateConstIterator(); Object; Object++)
	{
		auto Graph = Cast<UBP_Graph_Template>(*Object);
		if (Graph != nullptr)
		{
			TSharedRef<FGraphEditorToolkit_Template>EditorToolkit = MakeShareable(new FGraphEditorToolkit_Template());
			EditorToolkit->InitGraphAssetEditor(Mode, EditWithinLevelEditor, Graph);
		}
	}
}

#undef LOCTEXT_NAMESPACE