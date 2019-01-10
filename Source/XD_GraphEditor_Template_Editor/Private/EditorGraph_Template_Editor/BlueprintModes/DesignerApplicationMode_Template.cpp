// Fill out your copyright notice in the Description page of Project Settings.

#include "DesignerApplicationMode_Template.h"
#include "WorkflowTabFactory.h"
#include "GraphEditorToolkit_Template.h"
#include "BlueprintEditor.h"
#include "PropertyEditorModule.h"
#include "GraphEditor.h"
#include "EditorGraph_Template.h"

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
	TabLabel = LOCTEXT("DesingerDetails_Template_TabLabel", "细节");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DesingerDetails_Template_ViewMenu_Desc", "细节");
	ViewMenuTooltip = LOCTEXT("DesingerDetails_Template_ViewMenu_ToolTip", "Show the Details");
}

TSharedRef<SWidget> FDesignerDetailsSummoner_Template::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	class SDetails_TemplateView : public SCompoundWidget, public FNotifyHook
	{
	public:
		SLATE_BEGIN_ARGS(SDetails_TemplateView) {}
		SLATE_END_ARGS()
	public:
		TSharedPtr<SEditableTextBox> NameTextBox;
		TSharedPtr<class IDetailsView> PropertyView;

		void Construct(const FArguments& InArgs)
		{
			FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

			FNotifyHook* NotifyHook = this;

			FDetailsViewArgs DetailsViewArgs(
				/*bUpdateFromSelection=*/ false,
				/*bLockable=*/ false,
				/*bAllowSearch=*/ true,
				FDetailsViewArgs::HideNameArea,
				/*bHideSelectionTip=*/ true,
				/*InNotifyHook=*/ NotifyHook,
				/*InSearchInitialKeyFocus=*/ false,
				/*InViewIdentifier=*/ NAME_None);
			DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Automatic;

			PropertyView = EditModule.CreateDetailView(DetailsViewArgs);

			ChildSlot
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 6)
				[
					SNew(SHorizontalBox)
					.Visibility(this, &SDetails_TemplateView::GetNameAreaVisibility)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0, 0, 3, 0)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(this, &SDetails_TemplateView::GetNameIcon)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0, 0, 6, 0)
					[
						SNew(SBox)
						.WidthOverride(200.0f)
						.VAlign(VAlign_Center)
						[
							SAssignNew(NameTextBox, SEditableTextBox)
							.SelectAllTextWhenFocused(true)
							.HintText(LOCTEXT("Name", "Name"))
							.Text(this, &SDetails_TemplateView::GetNameText)
							.OnTextChanged(this, &SDetails_TemplateView::HandleNameTextChanged)
							.OnTextCommitted(this, &SDetails_TemplateView::HandleNameTextCommitted)
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SCheckBox)
						.IsChecked(this, &SDetails_TemplateView::GetIsVariable)
						.OnCheckStateChanged(this, &SDetails_TemplateView::HandleIsVariableChanged)
						.Padding(FMargin(3,1,3,1))
						[
							SNew(STextBlock)
							.Text(LOCTEXT("IsVariable", "Is Variable"))
						]
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					PropertyView.ToSharedRef()
				]
			];
		}

		EVisibility GetNameAreaVisibility() const { return EVisibility::Visible; }

		const FSlateBrush* GetNameIcon() const { return nullptr; }

		FText GetNameText() const { return FText::GetEmpty(); }

		bool HandleVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage) { return true; }

		void HandleNameTextChanged(const FText& Text){}
		void HandleNameTextCommitted(const FText& Text, ETextCommit::Type CommitType){}

		ECheckBoxState GetIsVariable() const { return ECheckBoxState::Checked; }
		void HandleIsVariableChanged(ECheckBoxState CheckState) {}
	};
	return SNew(SDetails_TemplateView);
}

struct FDesignerGraphSummoner_Template : public FWorkflowTabFactory
{
public:
	FDesignerGraphSummoner_Template(TSharedPtr<class FGraphEditorToolkit_Template> InBlueprintEditor);

	TSharedPtr<class FGraphEditorToolkit_Template> InBlueprintEditor;

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FDesignerGraphSummoner_Template::FDesignerGraphSummoner_Template(TSharedPtr<class FGraphEditorToolkit_Template> InBlueprintEditor)
	:FWorkflowTabFactory(FDesignerApplicationMode_Template::GraphTabId, InBlueprintEditor),
	InBlueprintEditor(InBlueprintEditor)
{
	TabLabel = LOCTEXT("DesingerGraph_Template_TabLabel", "图表");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DesingerGraph_Template_ViewMenu_Desc", "图表");
	ViewMenuTooltip = LOCTEXT("DesingerGraph_Template_ViewMenu_ToolTip", "Show the Details");
}

TSharedRef<SWidget> FDesignerGraphSummoner_Template::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SNew(SGraphEditor)
		.AdditionalCommands(InBlueprintEditor->DesignerEditorCommands)
		.GraphToEdit(InBlueprintEditor->DesignerGraph_Template->EdGraph);
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