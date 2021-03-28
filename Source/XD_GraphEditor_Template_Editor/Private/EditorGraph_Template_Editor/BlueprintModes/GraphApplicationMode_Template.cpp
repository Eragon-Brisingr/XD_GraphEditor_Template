// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorGraph_Template_Editor/BlueprintModes/GraphApplicationMode_Template.h"
#include <BlueprintEditorTabs.h>

#include "EditorGraph_Template_Editor/Toolkits/GraphEditor_Template.h"

FGraphApplicationMode_Template::FGraphApplicationMode_Template(TSharedPtr<class FGraphEditor_Template> GraphEditorToolkit)
	: FBlueprintApplicationModeTemplate(GraphEditorToolkit, FBlueprintApplicationModesTemplate::GraphMode)
{
	TabLayout = FTabManager::NewLayout("Graph_Template_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.186721f)
				->SetHideTabWell(true)
				->AddTab(GraphEditorToolkit->GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.15f)
					->Split
					(
						FTabManager::NewStack()->SetSizeCoefficient(0.5f)
						->AddTab(FBlueprintEditorTabs::MyBlueprintID, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()->SetSizeCoefficient(0.5f)
						->AddTab(FBlueprintEditorTabs::DetailsID, ETabState::OpenedTab)
					)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.70f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.80f)
						->AddTab("Document", ETabState::ClosedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.20f)
						->AddTab(FBlueprintEditorTabs::CompilerResultsID, ETabState::ClosedTab)
						->AddTab(FBlueprintEditorTabs::FindResultsID, ETabState::ClosedTab)
					)
				)
				->Split
				(
					FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
					->SetSizeCoefficient(0.15f)
					->Split
					(
						FTabManager::NewStack()
						->AddTab(FBlueprintEditorTabs::PaletteID, ETabState::ClosedTab)
					)
				)
			)
		);

	AddModeSwitchToolBarExtension();
}

void FGraphApplicationMode_Template::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	FBlueprintApplicationModeTemplate::RegisterTabFactories(InTabManager);
}

void FGraphApplicationMode_Template::PostActivateMode()
{
	FBlueprintApplicationModeTemplate::PostActivateMode();
}
