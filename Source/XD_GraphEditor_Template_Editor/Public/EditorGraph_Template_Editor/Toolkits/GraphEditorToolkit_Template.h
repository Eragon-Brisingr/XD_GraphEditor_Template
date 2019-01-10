// Copyright (c) 2018 Jhonny Hueller
#pragma once

#include "CoreMinimal.h"
#include "BlueprintEditor.h"
#include "NotifyHook.h"
#include "GraphEditor.h"
#include "IDetailsView.h"
#include "BP_Graph_Template.h"

class UEditorGraph_Blueprint_Template;

/**
 * 
 */
class FGraphEditorToolkit_Template : public FBlueprintEditor
{
public:
	FGraphEditorToolkit_Template();
	~FGraphEditorToolkit_Template();

	// Inherited via FAssetEditorToolkit
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager) override;
	virtual void SaveAsset_Execute() override;

	virtual void InitGarph_TemplateEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UEditorGraph_Blueprint_Template* InBP);
	virtual void BlueprintCompiled();

	// Begin FBlueprintEditor
	void InitalizeExtenders() override;
	virtual void RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated = false) override;
	virtual FGraphAppearanceInfo GetGraphAppearance(class UEdGraph* InGraph) const override;
	virtual void AppendExtraCompilerResults(TSharedPtr<class IMessageLogListing> ResultsListing) override;
	virtual TSubclassOf<UEdGraphSchema> GetDefaultSchemaClass() const override;
	// End FBlueprintEditor

	class UEditorGraph_Blueprint_Template* GetTemplateBlueprintObj() const;

	TSharedPtr<FUICommandList> DesignerEditorCommands;
	UBP_Graph_Template* DesignerGraph_Template;
private:
	TSharedPtr<SGraphEditor> EdGraphEditor;
	TSharedPtr<IDetailsView> DetailsWidget;

	FGraphPanelSelectionSet GetSelectedNodes();
	TSharedRef<SDockTab> HandleTabManagerSpawnTabDetails(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> HandleTabManagerSpawnTabGraph(const FSpawnTabArgs& Args);
	void BindToolkitCommands();

	//Delegates
	void OnDesignerCommandSelectAllNodes();
	bool CanDesignerSelectAllNodes();

	void OnDesignerCommandCopy();
	bool CanDesignerCopyNodes();

	void OnDesignerCommandPaste();
	bool CanDesignerPasteNodes();

	void OnDesignerCommandCut();
	bool CanDesignerCutNodes();

	void OnDesignerCommandDuplicate();
	bool CanDesignerDuplicateNodes();

	void OnDesignerCommandDelete();
	bool CanDesignerDeleteNodes();

};
