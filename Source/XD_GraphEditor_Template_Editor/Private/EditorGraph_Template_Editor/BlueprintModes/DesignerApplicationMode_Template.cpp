// Fill out your copyright notice in the Description page of Project Settings.

#include "DesignerApplicationMode_Template.h"
#include "WorkflowTabFactory.h"
#include "GraphEditorToolkit_Template.h"
#include "BlueprintEditor.h"
#include "PropertyEditorModule.h"
#include "GraphEditor.h"
#include "GenericCommands.h"
#include "EdGraphUtilities.h"
#include "PlatformApplicationMisc.h"
#include "SBlueprintEditorToolbar.h"
#include "BlueprintEditorUtils.h"
#include "Editor_GraphNode_Template.h"
#include "EditorGraph_Template.h"
#include "PropertyEditorDelegates.h"
#include "SKismetInspector.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "SComboButton.h"
#include "DetailWidgetRow.h"
#include "ObjectEditorUtils.h"
#include "KismetEditorUtilities.h"
#include "SWidgetSwitcher.h"
#include "K2Node_ComponentBoundEvent.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

const FName FDesignerApplicationMode_Template::DetailsTabId(TEXT("Designer_Template_DetailsTabId"));
const FName FDesignerApplicationMode_Template::GraphTabId(TEXT("Designer_Template_GraphTabId"));

class SDetails_TemplateView : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SDetails_TemplateView) {}
	SLATE_END_ARGS()
public:
	TSharedPtr<SEditableTextBox> NameTextBox;
	TSharedPtr<class IDetailsView> PropertyView;

	TWeakPtr<FGraphEditorToolkit_Template> Editor;

	void Construct(const FArguments& InArgs, TWeakPtr<FGraphEditorToolkit_Template> InEditor)
	{
		Editor = InEditor;

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

		class FDesignerDelegate_Template : public IDetailCustomization
		{
			FDesignerDelegate_Template(UEditorGraph_Blueprint_Template* Blueprint)
				:Blueprint(Blueprint)
			{}

			UEditorGraph_Blueprint_Template* Blueprint;

			void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
			{
				TArray< TWeakObjectPtr<UObject> > OutObjects;
				DetailLayout.GetObjectsBeingCustomized(OutObjects);

				if (OutObjects.Num() == 1)
				{
					UClass* PropertyClass = OutObjects[0].Get()->GetClass();

					for (TFieldIterator<UProperty> PropertyIt(PropertyClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
					{
						UProperty* Property = *PropertyIt;

						if (UMulticastDelegateProperty* MulticastDelegateProperty = Cast<UMulticastDelegateProperty>(Property))
						{
							CreateMulticastEventCustomization(DetailLayout, OutObjects[0].Get()->GetFName(), PropertyClass, MulticastDelegateProperty);
						}
					}
				}
			}

			void CreateMulticastEventCustomization(IDetailLayoutBuilder& DetailLayout, FName ThisComponentName, UClass* PropertyClass, UMulticastDelegateProperty* DelegateProperty)
			{
				const FString AddString = FString(TEXT("Add "));
				const FString ViewString = FString(TEXT("View "));

				const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

				if ( !K2Schema->CanUserKismetAccessVariable(DelegateProperty, PropertyClass, UEdGraphSchema_K2::MustBeDelegate) )
				{
					return;
				}

				FText PropertyTooltip = DelegateProperty->GetToolTipText();
				if ( PropertyTooltip.IsEmpty() )
				{
					PropertyTooltip = FText::FromString(DelegateProperty->GetName());
				}

				UObjectProperty* ComponentProperty = FindField<UObjectProperty>(Blueprint->SkeletonGeneratedClass, ThisComponentName);

				if (!ComponentProperty)
				{
					return;
				}

				FName PropertyName = ComponentProperty->GetFName();
				FName EventName = DelegateProperty->GetFName();
				FText EventText = DelegateProperty->GetDisplayNameText();

				IDetailCategoryBuilder& EventCategory = DetailLayout.EditCategory(TEXT("Events"), LOCTEXT("Events", "Events"), ECategoryPriority::Uncommon);

				EventCategory.AddCustomRow(EventText)
					.NameContent()
					[
						SNew(SHorizontalBox)
						.ToolTipText(DelegateProperty->GetToolTipText())
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0, 0, 5, 0)
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("GraphEditor.Event_16x"))
						]

						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Font(IDetailLayoutBuilder::GetDetailFont())
							.Text(EventText)
						]
					]
					.ValueContent()
					.MinDesiredWidth(150)
					.MaxDesiredWidth(200)
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
						.HAlign(HAlign_Center)
						.OnClicked(this, &FDesignerDelegate_Template::HandleAddOrViewEventForVariable, EventName, PropertyName, MakeWeakObjectPtr(PropertyClass))
						.ForegroundColor(FSlateColor::UseForeground())
						[
							SNew(SWidgetSwitcher)
							.WidgetIndex(this, &FDesignerDelegate_Template::HandleAddOrViewIndexForButton, EventName, PropertyName)
							+ SWidgetSwitcher::Slot()
							[
								SNew(STextBlock)
								.Font(FEditorStyle::GetFontStyle(TEXT("BoldFont")))
								.Text(LOCTEXT("ViewEvent", "View"))
							]
							+ SWidgetSwitcher::Slot()
							[
								SNew(SImage)
								.Image(FEditorStyle::GetBrush("Plus"))
							]
						]
					];
			}

			FReply HandleAddOrViewEventForVariable(const FName EventName, FName PropertyName, TWeakObjectPtr<UClass> PropertyClass)
			{
				UBlueprint* BlueprintObj = Blueprint;

				// Find the corresponding variable property in the Blueprint
				UObjectProperty* VariableProperty = FindField<UObjectProperty>(BlueprintObj->SkeletonGeneratedClass, PropertyName);

				if (VariableProperty)
				{
					if (!FKismetEditorUtilities::FindBoundEventForComponent(BlueprintObj, EventName, VariableProperty->GetFName()))
					{
						FKismetEditorUtilities::CreateNewBoundEventForClass(PropertyClass.Get(), EventName, BlueprintObj, VariableProperty);
					}
					else
					{
						const UK2Node_ComponentBoundEvent* ExistingNode = FKismetEditorUtilities::FindBoundEventForComponent(BlueprintObj, EventName, VariableProperty->GetFName());
						if (ExistingNode)
						{
							FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(ExistingNode);
						}
					}
				}

				return FReply::Handled();
			}

			int32 HandleAddOrViewIndexForButton(const FName EventName, FName PropertyName) const
			{
				UBlueprint* BlueprintObj = Blueprint;

				if (FKismetEditorUtilities::FindBoundEventForComponent(BlueprintObj, EventName, PropertyName))
				{
					return 0; // View
				}

				return 1; // Add
			}
		public:
			static TSharedRef<IDetailCustomization> MakeInstance(UEditorGraph_Blueprint_Template* Blueprint)
			{
				return MakeShareable(new FDesignerDelegate_Template(Blueprint));
			}
		};

		PropertyView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDesignerDelegate_Template::MakeInstance, Editor.Pin()->GetTemplateBlueprintObj()));

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
				.HAlign(HAlign_Fill)
				.Padding(0, 0, 6, 0)
				[
					SAssignNew(NameTextBox, SEditableTextBox)
					.SelectAllTextWhenFocused(true)
					.HintText(LOCTEXT("Name", "Name"))
					.Text(this, &SDetails_TemplateView::GetNameText)
					.OnTextChanged(this, &SDetails_TemplateView::HandleNameTextChanged)
					.OnTextCommitted(this, &SDetails_TemplateView::HandleNameTextCommitted)
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
						.Text(LOCTEXT("蓝图可见", "蓝图可见"))
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

	FText GetNameText() const
	{
		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		if (Selections.Num() == 1)
		{
			if (UObject* Obj = Selections[0].Get())
			{
				return FText::FromString(Obj->GetName());
			}
		}

		return FText::GetEmpty();
	}

	bool HandleVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
	{
		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		if (Selections.Num() == 1)
		{
			if (UObject* Obj = Selections[0].Get())
			{
				if (Obj->Rename(*InText.ToString(), nullptr, REN_Test))
				{
					return true;
				}
				else
				{
					OutErrorMessage = LOCTEXT("Rename Error", "存在重名对象，无法重命名");
					return false;
				}
			}
		}
		return false;
	}

	void HandleNameTextChanged(const FText& Text)
	{
		FText OutErrorMessage;
		if (!HandleVerifyNameTextChanged(Text, OutErrorMessage))
		{
			NameTextBox->SetError(OutErrorMessage);
		}
		else
		{
			NameTextBox->SetError(FText::GetEmpty());
		}
	}

	void HandleNameTextCommitted(const FText& Text, ETextCommit::Type CommitType)
	{
		UEditorGraph_Blueprint_Template* Blueprint = Editor.Pin()->GetTemplateBlueprintObj();

		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		if (Selections.Num() == 1)
		{
			if (UBP_GraphNode_Template* Node = Cast<UBP_GraphNode_Template>(Selections[0].Get()))
			{
				const TCHAR* NewName = *Text.ToString();

				if (Node->Rename(NewName, nullptr, REN_Test))
				{
					if (Node->bIsVariable)
					{
						FBlueprintEditorUtils::RenameMemberVariable(Blueprint, Node->GetFName(), NewName);
					}
					Node->Rename(NewName);

					Editor.Pin()->GetEditorGraph()->RefreshNodes();
				}
			}
		}

		if (CommitType == ETextCommit::OnUserMovedFocus || CommitType == ETextCommit::OnCleared)
		{
			NameTextBox->SetError(FText::GetEmpty());
		}
	}

	ECheckBoxState GetIsVariable() const 
	{
		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		if (Selections.Num() == 1)
		{
			if (UBP_GraphNode_Template* Node = Cast<UBP_GraphNode_Template>(Selections[0].Get()))
			{
				return Node->bIsVariable ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			}
		}
		return ECheckBoxState::Unchecked; 
	}

	void HandleIsVariableChanged(ECheckBoxState CheckState)
	{
		const FScopedTransaction Transaction(LOCTEXT("VariableToggle", "Variable Toggle"));

		UEditorGraph_Blueprint_Template* Blueprint = Editor.Pin()->GetTemplateBlueprintObj();

		const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
		for (TWeakObjectPtr<UObject> Obj : Selections)
		{
			if (UBP_GraphNode_Template* Node = Cast<UBP_GraphNode_Template>(Obj.Get()))
			{
				if (CheckState == ECheckBoxState::Checked)
				{
					if (Node->bIsVariable == false)
					{
						if (FBlueprintEditorUtils::AddMemberVariable(Blueprint, Node->GetFName(), FEdGraphPinType(UEdGraphSchema_K2::PC_Object, UEdGraphSchema_K2::PC_Object, Node->GetClass(), EPinContainerType::None, false, FEdGraphTerminalType())))
						{
							Node->bIsVariable = true;
						}
					}
				}
				else
				{
					if (Node->bIsVariable == true)
					{
						FBlueprintEditorUtils::RemoveMemberVariable(Blueprint, Node->GetFName());
						Node->bIsVariable = false;
					}
				}
			}
		}

		PropertyView->ForceRefresh();
	}
};

struct FDesignerDetailsSummoner_Template : public FWorkflowTabFactory
{
public:
	FDesignerDetailsSummoner_Template(class FDesignerApplicationMode_Template* DesignerApplicationMode, TSharedPtr<class FGraphEditorToolkit_Template> InDesignGraphEditor);

	FDesignerApplicationMode_Template* DesignerApplicationMode;
	TWeakPtr<class FGraphEditorToolkit_Template> InDesignGraphEditor;

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FDesignerDetailsSummoner_Template::FDesignerDetailsSummoner_Template(class FDesignerApplicationMode_Template* DesignerApplicationMode, TSharedPtr<class FGraphEditorToolkit_Template> InDesignGraphEditor)
	: FWorkflowTabFactory(FDesignerApplicationMode_Template::DetailsTabId, InDesignGraphEditor),
	DesignerApplicationMode(DesignerApplicationMode),
	InDesignGraphEditor(InDesignGraphEditor)
{
	TabLabel = LOCTEXT("DesingerDetails_Template_TabLabel", "细节");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DesingerDetails_Template_ViewMenu_Desc", "细节");
	ViewMenuTooltip = LOCTEXT("DesingerDetails_Template_ViewMenu_ToolTip", "Show the Details");
}

TSharedRef<SWidget> FDesignerDetailsSummoner_Template::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedRef<SDetails_TemplateView> DesignerDetails = SNew(SDetails_TemplateView, InDesignGraphEditor);
	DesignerApplicationMode->DesignerDetails = DesignerDetails;
	return DesignerDetails;
}

struct FDesignerGraphSummoner_Template : public FWorkflowTabFactory
{
public:
	FDesignerGraphSummoner_Template(class FDesignerApplicationMode_Template* DesignerApplicationMode, TSharedPtr<class FGraphEditorToolkit_Template> InDesignGraphEditor);

	FDesignerApplicationMode_Template* DesignerApplicationMode;
	TWeakPtr<class FGraphEditorToolkit_Template> InDesignGraphEditor;

	TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;
};

FDesignerGraphSummoner_Template::FDesignerGraphSummoner_Template(class FDesignerApplicationMode_Template* DesignerApplicationMode, TSharedPtr<class FGraphEditorToolkit_Template> InDesignGraphEditor)
	:FWorkflowTabFactory(FDesignerApplicationMode_Template::GraphTabId, InDesignGraphEditor),
	DesignerApplicationMode(DesignerApplicationMode),
	InDesignGraphEditor(InDesignGraphEditor)
{
	TabLabel = LOCTEXT("DesingerGraph_Template_TabLabel", "图表");
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), "PhysicsAssetEditor.Tabs.Graph");

	bIsSingleton = true;

	ViewMenuDescription = LOCTEXT("DesingerGraph_Template_ViewMenu_Desc", "图表");
	ViewMenuTooltip = LOCTEXT("DesingerGraph_Template_ViewMenu_ToolTip", "Show the Garph");
}

TSharedRef<SWidget> FDesignerGraphSummoner_Template::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	return SAssignNew(DesignerApplicationMode->DesignerGraphEditor, SGraphEditor)
		.AdditionalCommands(DesignerApplicationMode->DesignerEditorCommands)
		.GraphToEdit(InDesignGraphEditor.Pin()->DesignerGraph_Template->EdGraph)
		.GraphEvents(DesignerApplicationMode->DesignerGraphEvents);
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
					->SetSizeCoefficient(0.8f)
					->AddTab(GraphTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.2f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
			)
		);

 	TabFactories.RegisterFactory(MakeShareable(new FDesignerDetailsSummoner_Template(this, GraphEditorToolkit)));
 	TabFactories.RegisterFactory(MakeShareable(new FDesignerGraphSummoner_Template(this, GraphEditorToolkit)));

	ToolbarExtender = MakeShareable(new FExtender);
	GraphEditorToolkit->GetToolbarBuilder()->AddCompileToolbar(ToolbarExtender);
	GraphEditorToolkit->GetToolbarBuilder()->AddDebuggingToolbar(ToolbarExtender);
	AddModeSwitchToolBarExtension();

	BindDesignerToolkitCommands();
	DesignerGraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateRaw(this, &FDesignerApplicationMode_Template::HandleSelectionChanged);
}

void FDesignerApplicationMode_Template::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	FGraphEditorToolkit_Template *BP = GetBlueprintEditor();

	BP->RegisterToolbarTab(InTabManager.ToSharedRef());
	BP->PushTabFactories(TabFactories);
}

void FDesignerApplicationMode_Template::PreDeactivateMode()
{

}

void FDesignerApplicationMode_Template::PostActivateMode()
{

}

void FDesignerApplicationMode_Template::HandleSelectionChanged(const FGraphPanelSelectionSet& SelectionSet)
{
	if (DesignerDetails.IsValid())
	{
		TSharedPtr<SDetails_TemplateView> Details = DesignerDetails.Pin();

		TArray<UObject*> ShowObjects;
		for (UObject* Obj : SelectionSet)
		{
			if (UEditor_GraphNode_Template* EdGraphNode_Template = Cast<UEditor_GraphNode_Template>(Obj))
			{
				if (EdGraphNode_Template->BP_Node_Template)
				{
					ShowObjects.Add(EdGraphNode_Template->BP_Node_Template);
				}
			}
		}

		Details->PropertyView->SetObjects(ShowObjects, true);
	}
}

void FDesignerApplicationMode_Template::BindDesignerToolkitCommands()
{
	if (!DesignerEditorCommands.IsValid())
	{
		DesignerEditorCommands = MakeShareable(new FUICommandList());

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().SelectAll,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandSelectAllNodes),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerSelectAllNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Cut,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandCut),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerCutNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Copy,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandCopy),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerCopyNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Paste,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandPaste),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerPasteNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandDuplicate),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerDuplicateNodes)
		);

		DesignerEditorCommands->MapAction
		(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::OnDesignerCommandDelete),
			FCanExecuteAction::CreateRaw(this, &FDesignerApplicationMode_Template::CanDesignerDeleteNodes)
		);

	}
}

FGraphPanelSelectionSet FDesignerApplicationMode_Template::GetSelectedNodes()
{
	return GetDesignerGraphEditor()->GetSelectedNodes();
}

void FDesignerApplicationMode_Template::OnDesignerCommandSelectAllNodes()
{
	if (DesignerGraphEditor.IsValid())
	{
		GetDesignerGraphEditor()->SelectAllNodes();
	}
}

bool FDesignerApplicationMode_Template::CanDesignerSelectAllNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandCut()
{
	OnDesignerCommandCopy();

	const FGraphPanelSelectionSet OldSelectedNodes = GetDesignerGraphEditor()->GetSelectedNodes();
	GetDesignerGraphEditor()->ClearSelectionSet();
	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
		if (Node && Node->CanDuplicateNode())
		{
			GetDesignerGraphEditor()->SetNodeSelection(Node, true);
		}
	}

	OnDesignerCommandDelete();

	GetDesignerGraphEditor()->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(OldSelectedNodes); It; ++It)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*It);
		if (Node)
			GetDesignerGraphEditor()->SetNodeSelection(Node, true);
	}
}

bool FDesignerApplicationMode_Template::CanDesignerCutNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandCopy()
{
	FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	FString ExportedText;

	for (FGraphPanelSelectionSet::TIterator it(SelectedNodes); it; ++it)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(*it);
		if (Node)
			Node->PrepareForCopying();
		else
			it.RemoveCurrent();
	}

	FEdGraphUtilities::ExportNodesToText(SelectedNodes, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);

	for (FGraphPanelSelectionSet::TIterator it(SelectedNodes); it; ++it)
	{
		UEditor_GraphNode_Template* Node = Cast<UEditor_GraphNode_Template>(*it);
		if (Node)
			Node->PostCopyNode();
	}
}

bool FDesignerApplicationMode_Template::CanDesignerCopyNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandPaste()
{
	const FVector2D PasteLocation = GetDesignerGraphEditor()->GetPasteLocation();

	UEdGraph* EdGraph = GetDesignerGraphEditor()->GetCurrentGraph();
	EdGraph->Modify();
	GetDesignerGraphEditor()->ClearSelectionSet();

	FString ExportedText;
	FPlatformApplicationMisc::ClipboardPaste(ExportedText);
	TSet<UEdGraphNode*> ImportedNodes;
	FEdGraphUtilities::ImportNodesFromText(EdGraph, ExportedText, ImportedNodes);

	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
	{
		UEditor_GraphNode_Template* Node = Cast<UEditor_GraphNode_Template>(*It);
		MyEditor_Template.Pin()->DesignerGraph_Template->AddNode(Node->BP_Node_Template);
	}

	FVector2D AvgNodePosition(0.0f, 0.0f);

	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		AvgNodePosition.X += Node->NodePosX;
		AvgNodePosition.Y += Node->NodePosY;
	}

	float InvNumNodes = 1.0f / float(ImportedNodes.Num());
	AvgNodePosition.X *= InvNumNodes;
	AvgNodePosition.Y *= InvNumNodes;

	for (TSet<UEdGraphNode*>::TIterator It(ImportedNodes); It; ++It)
	{
		UEdGraphNode* Node = *It;
		GetDesignerGraphEditor()->SetNodeSelection(Node, true);

		Node->NodePosX = (Node->NodePosX - AvgNodePosition.X) + PasteLocation.X;
		Node->NodePosY = (Node->NodePosY - AvgNodePosition.Y) + PasteLocation.Y;

		Node->SnapToGrid(16);

		// Give new node a different Guid from the old one
		Node->CreateNewGuid();
	}

	GetDesignerGraphEditor()->NotifyGraphChanged();

	UObject* GraphOwner = EdGraph->GetOuter();
	if (GraphOwner)
	{
		GraphOwner->PostEditChange();
		GraphOwner->MarkPackageDirty();
	}

}

bool FDesignerApplicationMode_Template::CanDesignerPasteNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandDuplicate()
{
	OnDesignerCommandCopy();
	OnDesignerCommandPaste();
}

bool FDesignerApplicationMode_Template::CanDesignerDuplicateNodes()
{
	return true;
}

void FDesignerApplicationMode_Template::OnDesignerCommandDelete()
{
	GetDesignerGraphEditor()->GetCurrentGraph()->Modify();

	const FGraphPanelSelectionSet SelectedNodes = GetSelectedNodes();
	GetDesignerGraphEditor()->ClearSelectionSet();

	for (FGraphPanelSelectionSet::TConstIterator It(SelectedNodes); It; ++It)
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(*It))
		{
			Node->Modify();
			Node->DestroyNode();
		}
	}
}

bool FDesignerApplicationMode_Template::CanDesignerDeleteNodes()
{
	return true;
}

#undef LOCTEXT_NAMESPACE