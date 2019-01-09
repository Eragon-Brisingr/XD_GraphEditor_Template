// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintApplicationModeTemplate.h"

/**
 * 
 */
class XD_GRAPHEDITOR_TEMPLATE_EDITOR_API FGraphApplicationMode_Template : public FBlueprintApplicationModeTemplate
{
public:
	FGraphApplicationMode_Template(TSharedPtr<class FGraphEditorToolkit_Template> GraphEditorToolkit);

	// FApplicationMode interface
	void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	void PostActivateMode() override;
	// End of FApplicationMode interface
};
