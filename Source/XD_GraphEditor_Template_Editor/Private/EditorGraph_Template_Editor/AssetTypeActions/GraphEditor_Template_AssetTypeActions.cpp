// Fill out your copyright notice in the Description page of Project Settings.

#include "GraphEditor_Template_AssetTypeActions.h"
#include "XD_GraphEditor_Template_Editor.h"
#include "GraphEditor_Template.h"
#include "ModuleManager.h"
#include "EditorGraph_Blueprint_Template.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template_AssetTypeActions"

FGraphEditor_Template_AssetTypeActions::FGraphEditor_Template_AssetTypeActions()
{
}

FText FGraphEditor_Template_AssetTypeActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "GraphTypeActions", "EditorGraph_Blueprint_Template");
}

UClass * FGraphEditor_Template_AssetTypeActions::GetSupportedClass() const
{
	return UEditorGraph_Blueprint_Template::StaticClass();
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
		auto CustomBP = Cast<UEditorGraph_Blueprint_Template>(*Object);
		if (CustomBP != nullptr)
		{
			TSharedRef<FGraphEditor_Template> EditorToolkit = MakeShareable(new FGraphEditor_Template());
			EditorToolkit->InitGarph_TemplateEditor(Mode, EditWithinLevelEditor, CustomBP);
		}
	}
}

#undef LOCTEXT_NAMESPACE