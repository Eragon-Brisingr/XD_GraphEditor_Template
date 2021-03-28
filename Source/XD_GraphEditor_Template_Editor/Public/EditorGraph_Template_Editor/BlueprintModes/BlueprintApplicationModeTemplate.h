// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintEditorModes.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class UEditorGraph_Blueprint_Template;

/**
 * 
 */
struct XD_GRAPHEDITOR_TEMPLATE_EDITOR_API FBlueprintApplicationModesTemplate
{
	// Mode constants
	static const FName DesignerMode;
	static const FName GraphMode;

	static FText GetLocalizedMode(const FName InMode);

private:
	FBlueprintApplicationModesTemplate() {}
};

class XD_GRAPHEDITOR_TEMPLATE_EDITOR_API FBlueprintApplicationModeTemplate : public FBlueprintEditorApplicationMode
{
public:
	FBlueprintApplicationModeTemplate(TSharedPtr<class FGraphEditor_Template> GraphEditorToolkit, FName InModeName);

	void AddModeSwitchToolBarExtension();

protected:
	UEditorGraph_Blueprint_Template* GetBlueprint() const;

	class FGraphEditor_Template* GetBlueprintEditor() const;

	TWeakPtr<class FGraphEditor_Template> MyEditor_Template;

	// Set of spawnable tabs in the mode
	FWorkflowAllowedTabSet TabFactories;
};
