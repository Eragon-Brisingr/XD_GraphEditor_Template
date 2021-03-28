// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "BlueprintEditor.h"
#include "GraphEditor.h"

class UEditorGraph_Blueprint_Template;
class UEditorGraph_Template;
class UBP_Graph_Template;

/**
 * 
 */
class FGraphEditor_Template : public FBlueprintEditor
{
	using Super = FBlueprintEditor;
public:
	FGraphEditor_Template();
	~FGraphEditor_Template();

	// Inherited via FAssetEditorToolkit
	FLinearColor GetWorldCentricTabColorScale() const override;
	FName GetToolkitFName() const override;
	FText GetBaseToolkitName() const override;
	FString GetWorldCentricTabPrefix() const override;
	void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void SaveAsset_Execute() override;

	// Begin FBlueprintEditor
	void InitalizeExtenders() override;
	void RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated = false) override;
	FGraphAppearanceInfo GetGraphAppearance(class UEdGraph* InGraph) const override;
	TSubclassOf<UEdGraphSchema> GetDefaultSchemaClass() const override;
	// End FBlueprintEditor

public:
	void InitGarph_TemplateEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UEditorGraph_Blueprint_Template* InBP);
	void BlueprintCompiled(class UBlueprint* Blueprint);

	class UEditorGraph_Blueprint_Template* GetTemplateBlueprintObj() const;

	TWeakObjectPtr<UBP_Graph_Template> DesignerGraph_Template;

	UEditorGraph_Template* GetDesignerGraph() const;
};
