// Fill out your copyright notice in the Description page of Project Settings.

#include "DesignerApplicationMode_Template.h"
#include "WorkflowTabFactory.h"
#include "GraphEditorToolkit_Template.h"
#include "BlueprintEditor.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

const FName FDesignerApplicationMode_Template::DetailsTabId(TEXT("Designer_Template_DetailsTabId"));
const FName FDesignerApplicationMode_Template::GraphTabId(TEXT("Designer_Template_GraphTabId"));

struct FDesignerDetailsSummoner_Template : public FWorkflowTabFactory
{
public:
	FDesignerDetailsSummoner_Template(TSharedPtr<class FGraphEditorToolkit_Template> InBlueprintEditor);

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FDesignerDetailsSummoner_Template::FDesignerDetailsSummoner_Template(TSharedPtr<class FGraphEditorToolkit_Template> InBlueprintEditor)
	: FWorkflowTabFactory(FDesignerApplicationMode_Template::DetailsTabId, InBlueprintEditor)
{
	TabLabel = LOCTEXT("DesingerDetails_Template_TabLabel", "Details");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DesingerDetails_Template_ViewMenu_Desc", "Details");
	ViewMenuTooltip = LOCTEXT("DesingerDetails_Template_ViewMenu_ToolTip", "Show the Details");
}

TSharedRef<SWidget> FDesignerDetailsSummoner_Template::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return FWorkflowTabFactory::CreateTabBody(Info);
}

struct FDesignerGraphSummoner_Template : public FWorkflowTabFactory
{
public:
	FDesignerGraphSummoner_Template(TSharedPtr<class FGraphEditorToolkit_Template> InBlueprintEditor);

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FDesignerGraphSummoner_Template::FDesignerGraphSummoner_Template(TSharedPtr<class FGraphEditorToolkit_Template> InBlueprintEditor)
	:FWorkflowTabFactory(FDesignerApplicationMode_Template::GraphTabId, InBlueprintEditor)
{
	TabLabel = LOCTEXT("DesingerGraph_Template_TabLabel", "Details");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DesingerGraph_Template_ViewMenu_Desc", "Details");
	ViewMenuTooltip = LOCTEXT("DesingerGraph_Template_ViewMenu_ToolTip", "Show the Details");
}

TSharedRef<SWidget> FDesignerGraphSummoner_Template::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return FWorkflowTabFactory::CreateTabBody(Info);
}

FDesignerApplicationMode_Template::FDesignerApplicationMode_Template(TSharedPtr<class FGraphEditorToolkit_Template> GraphEditorToolkit)
	:FBlueprintApplicationModeTemplate(GraphEditorToolkit, FBlueprintApplicationModesTemplate::DesignerMode)
{
	WorkspaceMenuCategory = FWorkspaceItem::NewGroup(LOCTEXT("WorkspaceMenu_Designer_Template", "Designer_Template"));

	TabLayout = FTabManager::NewLayout("Designer_Template_Layout_v1_1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.2f)
				->AddTab(GraphEditorToolkit->GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.8f)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.15f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.85f)
					->AddTab(GraphTabId, ETabState::OpenedTab)
				)
			)
		);

	TabFactories.RegisterFactory(MakeShareable(new FDesignerDetailsSummoner_Template(GraphEditorToolkit)));
	TabFactories.RegisterFactory(MakeShareable(new FDesignerGraphSummoner_Template(GraphEditorToolkit)));

// 	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("EditorGraph_TemplateToolkitWorkspaceMenu", "Graph Editor"));
// 	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();
// 
// 	TabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FGraphEditorToolkit_Template::HandleTabManagerSpawnTabDetails))
// 		.SetDisplayName(LOCTEXT("DetailsTab", "Details"))
// 		.SetGroup(WorkspaceMenuCategoryRef);
// 	TabManager->RegisterTabSpawner(GraphTabId, FOnSpawnTab::CreateSP(this, &FGraphEditorToolkit_Template::HandleTabManagerSpawnTabGraph))
// 		.SetDisplayName(LOCTEXT("GraphTab", "Graph Editor"))
// 		.SetGroup(WorkspaceMenuCategoryRef);
}

void FDesignerApplicationMode_Template::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FGraphEditorToolkit_Template> BP = GetBlueprintEditor();

	BP->RegisterToolbarTab(InTabManager.ToSharedRef());
	BP->PushTabFactories(TabFactories);
}

void FDesignerApplicationMode_Template::PreDeactivateMode()
{

}

void FDesignerApplicationMode_Template::PostActivateMode()
{

}

#undef LOCTEXT_NAMESPACE