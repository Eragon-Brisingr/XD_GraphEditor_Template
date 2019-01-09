// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintEditorModes.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

/**
 * 
 */
struct XD_GRAPHEDITOR_TEMPLATE_EDITOR_API FBlueprintApplicationModesTemplate
{
	// Mode constants
	static const FName DesignerMode;
	static const FName GraphMode;

	static FText GetLocalizedMode(const FName InMode)
	{
		static TMap< FName, FText > LocModes;

		if (LocModes.Num() == 0)
		{
			LocModes.Add(DesignerMode, NSLOCTEXT("TemplateBlueprintModes", "DesignerMode", "Designer"));
			LocModes.Add(GraphMode, NSLOCTEXT("TemplateBlueprintModes", "GraphMode", "Graph"));
		}

		check(InMode != NAME_None);
		const FText* OutDesc = LocModes.Find(InMode);
		check(OutDesc);

		return *OutDesc;
	}

private:
	FBlueprintApplicationModesTemplate() {}
};

 class XD_GRAPHEDITOR_TEMPLATE_EDITOR_API FBlueprintApplicationModeTemplate : public FBlueprintEditorApplicationMode
 {
 public:
 	FBlueprintApplicationModeTemplate(TSharedPtr<class FGraphEditorToolkit_Template> GraphEditorToolkit, FName InModeName);
 
 protected:
 	UEditorGraph_Blueprint_Template* GetBlueprint() const;
 
 	TSharedPtr<class FGraphEditorToolkit_Template> GetBlueprintEditor() const;
 
 	TWeakPtr<class FGraphEditorToolkit_Template> MyEditor_Template;
 
 	// Set of spawnable tabs in the mode
 	FWorkflowAllowedTabSet TabFactories;
 };
