// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorGraph_Template_Editor/BlueprintModes/BlueprintApplicationModeTemplate.h"

/**
 * 
 */
class XD_GRAPHEDITOR_TEMPLATE_EDITOR_API FGraphApplicationMode_Template : public FBlueprintApplicationModeTemplate
{
public:
	FGraphApplicationMode_Template(TSharedPtr<class FGraphEditor_Template> GraphEditorToolkit);

	// FApplicationMode interface
	void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	void PostActivateMode() override;
	// End of FApplicationMode interface
};
