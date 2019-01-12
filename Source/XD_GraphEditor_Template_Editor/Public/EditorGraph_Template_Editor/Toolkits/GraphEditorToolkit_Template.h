// Copyright (c) 2018 Jhonny Hueller
#pragma once

#include "CoreMinimal.h"
#include "BlueprintEditor.h"
#include "NotifyHook.h"
#include "GraphEditor.h"
#include "IDetailsView.h"
#include "BP_Graph_Template.h"

class UEditorGraph_Blueprint_Template;
class UEditorGraph_Template;

/**
 * 
 */
class FGraphEditorToolkit_Template : public FBlueprintEditor
{
public:
	FGraphEditorToolkit_Template();
	~FGraphEditorToolkit_Template();

	// Inherited via FAssetEditorToolkit
	FLinearColor GetWorldCentricTabColorScale() const override;
	FName GetToolkitFName() const override;
	FText GetBaseToolkitName() const override;
	FString GetWorldCentricTabPrefix() const override;
	void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	void SaveAsset_Execute() override;

	// Begin FBlueprintEditor
	void InitalizeExtenders() override;
	void RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated = false) override;
	FGraphAppearanceInfo GetGraphAppearance(class UEdGraph* InGraph) const override;
	void AppendExtraCompilerResults(TSharedPtr<class IMessageLogListing> ResultsListing) override;
	TSubclassOf<UEdGraphSchema> GetDefaultSchemaClass() const override;
	// End FBlueprintEditor

public:
	void InitGarph_TemplateEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UEditorGraph_Blueprint_Template* InBP);
	void BlueprintCompiled(class UBlueprint* Blueprint);

	class UEditorGraph_Blueprint_Template* GetTemplateBlueprintObj() const;

	UBP_Graph_Template* DesignerGraph_Template;

	UEditorGraph_Template* GetEditorGraph() const;
};
