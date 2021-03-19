// Copyright (c) 2018 Jhonny Hueller
#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Misc/NotifyHook.h"
#include "GraphEditor.h"
#include "IDetailsView.h"

class UBP_Graph_Template;

/**
 * 
 */
class FGraphEditorToolkit_Template : public FAssetEditorToolkit, public FNotifyHook
{
public:
	FGraphEditorToolkit_Template();
	~FGraphEditorToolkit_Template();

	// Inherited via FAssetEditorToolkit
	FLinearColor GetWorldCentricTabColorScale() const override;
	FName GetToolkitFName() const override;
	FText GetBaseToolkitName() const override;
	FString GetWorldCentricTabPrefix() const override;
	void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void SaveAsset_Execute() override;

	virtual void InitGraphAssetEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UBP_Graph_Template* InGraph);
	virtual void BlueprintCompiled();
private:

	static const FName DetailsTabId;
	static const FName GraphTabId;

	UBP_Graph_Template* GraphAsset;
	TSharedPtr<SGraphEditor> EdGraphEditor;
	TSharedPtr<IDetailsView> DetailsWidget;

	TSharedPtr<FUICommandList> GraphEditorCommands;

	FGraphPanelSelectionSet GetSelectedNodes();
	TSharedRef<SDockTab> HandleTabManagerSpawnTabDetails(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> HandleTabManagerSpawnTabGraph(const FSpawnTabArgs& Args);
	void BindToolkitCommands();

	//Delegates
	void OnCommandSelectAllNodes();
	bool CanSelectAllNodes();

	void OnCommandCopy();
	bool CanCopyNodes();

	void OnCommandPaste();
	bool CanPasteNodes();

	void OnCommandCut();
	bool CanCutNodes();

	void OnCommandDuplicate();
	bool CanDuplicateNodes();

	void OnCommandDelete();
	bool CanDeleteNodes();

};
