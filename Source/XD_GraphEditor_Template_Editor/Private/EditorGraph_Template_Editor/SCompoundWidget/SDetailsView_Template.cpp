// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorGraph_Template_Editor/SCompoundWidget/SDetailsView_Template.h"
#include <IDetailCustomization.h>
#include <DetailCategoryBuilder.h>
#include <DetailLayoutBuilder.h>
#include <Widgets/Input/SComboButton.h>
#include <DetailWidgetRow.h>
#include <ObjectEditorUtils.h>
#include <Kismet2/KismetEditorUtilities.h>
#include <Widgets/Layout/SWidgetSwitcher.h>
#include <IDetailPropertyExtensionHandler.h>
#include <Kismet2/BlueprintEditorUtils.h>
#include <ScopedTransaction.h>
#include <PropertyEditorModule.h>
#include <K2Node_ComponentBoundEvent.h>

#include "EditorGraph_Template/Blueprint/EditorGraph_Blueprint_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorGraph_Template.h"
#include "EditorGraph_Template_Editor/Toolkits/GraphEditor_Template.h"
#include "EditorGraph_Template/Nodes/BP_GraphNode_Template.h"
#include "EditorGraph_Template_Editor/SCompoundWidget/SPropertyBinding_Template.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

void SDetailsView_Template::Construct(const FArguments& InArgs, TWeakPtr<FGraphEditor_Template> InEditor)
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

	PropertyView->SetExtensionHandler(MakeShareable(new FDetailExtensionHandler_Template(Editor.Pin().Get())));

	class FDesignerDelegate_Template : public IDetailCustomization
	{
		FDesignerDelegate_Template(FGraphEditor_Template* InEditor, UEditorGraph_Blueprint_Template* Blueprint)
			:Editor(InEditor), Blueprint(Blueprint)
		{}

		FGraphEditor_Template* Editor;
		UEditorGraph_Blueprint_Template* Blueprint;

		void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
		{
			TArray< TWeakObjectPtr<UObject> > OutObjects;
			DetailLayout.GetObjectsBeingCustomized(OutObjects);

			if (OutObjects.Num() == 1)
			{
				UObject* Obj = OutObjects[0].Get();
				UClass* PropertyClass = Obj->GetClass();

				for (TFieldIterator<FProperty> PropertyIt(PropertyClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
				{
					FProperty* Property = *PropertyIt;
						
					if (FMulticastDelegateProperty* MulticastDelegateProperty = CastField<FMulticastDelegateProperty>(Property))
					{
						CreateMulticastEventCustomization(DetailLayout, OutObjects[0].Get()->GetFName(), PropertyClass, MulticastDelegateProperty);
					}
				}
			}
		}

		void CreateMulticastEventCustomization(IDetailLayoutBuilder& DetailLayout, FName ThisComponentName, UClass* PropertyClass, FMulticastDelegateProperty* DelegateProperty)
		{
			const FString AddString = FString(TEXT("添加 "));
			const FString ViewString = FString(TEXT("查看 "));

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

			FObjectProperty* ComponentProperty = FindFProperty<FObjectProperty>(Blueprint->SkeletonGeneratedClass, ThisComponentName);

			if (!ComponentProperty)
			{
				return;
			}

			FName PropertyName = ComponentProperty->GetFName();
			FName EventName = DelegateProperty->GetFName();
			FText EventText = DelegateProperty->GetDisplayNameText();

			IDetailCategoryBuilder& EventCategory = DetailLayout.EditCategory(TEXT("事件"), LOCTEXT("事件", "事件"), ECategoryPriority::Uncommon);

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
			FObjectProperty* VariableProperty = FindFProperty<FObjectProperty>(BlueprintObj->SkeletonGeneratedClass, PropertyName);

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
		static TSharedRef<IDetailCustomization> MakeInstance(FGraphEditor_Template* Editor, UEditorGraph_Blueprint_Template* Blueprint)
		{
			return MakeShareable(new FDesignerDelegate_Template(Editor, Blueprint));
		}
	};

	FGraphEditor_Template* BlueprintEditor = Editor.Pin().Get();
	PropertyView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDesignerDelegate_Template::MakeInstance, BlueprintEditor, BlueprintEditor->GetTemplateBlueprintObj()));

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 6)
		[
			SNew(SHorizontalBox)
			.Visibility(this, &SDetailsView_Template::GetNameAreaVisibility)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0, 0, 3, 0)
			.VAlign(VAlign_Center)
			[
				SNew(SImage)
				.Image(this, &SDetailsView_Template::GetNameIcon)
			]

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.Padding(0, 0, 6, 0)
			[
				SAssignNew(NameTextBox, SEditableTextBox)
				.SelectAllTextWhenFocused(true)
				.HintText(LOCTEXT("Name", "Name"))
				.Text(this, &SDetailsView_Template::GetNameText)
				.OnTextChanged(this, &SDetailsView_Template::HandleNameTextChanged)
				.OnTextCommitted(this, &SDetailsView_Template::HandleNameTextCommitted)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(this, &SDetailsView_Template::GetIsVariable)
				.OnCheckStateChanged(this, &SDetailsView_Template::HandleIsVariableChanged)
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

EVisibility SDetailsView_Template::GetNameAreaVisibility() const
{
	return EVisibility::Visible;
}

const FSlateBrush* SDetailsView_Template::GetNameIcon() const
{
	return nullptr;
}

FText SDetailsView_Template::GetNameText() const
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

bool SDetailsView_Template::HandleVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
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

void SDetailsView_Template::HandleNameTextChanged(const FText& Text)
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

void SDetailsView_Template::HandleNameTextCommitted(const FText& Text, ETextCommit::Type CommitType)
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
					FBlueprintEditorUtils::ReplaceVariableReferences(Blueprint, Node->GetFName(), NewName);
					FBlueprintEditorUtils::ValidateBlueprintChildVariables(Blueprint, NewName);
					FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
				}

				FDelegateEditorBinding_Template* Binding = Blueprint->Bindings.FindByPredicate([&](const FDelegateEditorBinding_Template& E) {return E.Object.Get() == Node; });

				Node->Rename(NewName);

				if (Binding)
				{
					Binding->Object = Node;
				}

				Editor.Pin()->GetDesignerGraph()->RefreshNodes();
			}
		}
	}

	if (CommitType == ETextCommit::OnUserMovedFocus || CommitType == ETextCommit::OnCleared)
	{
		NameTextBox->SetError(FText::GetEmpty());
	}
}

ECheckBoxState SDetailsView_Template::GetIsVariable() const
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

void SDetailsView_Template::HandleIsVariableChanged(ECheckBoxState CheckState)
{
	const FScopedTransaction Transaction(LOCTEXT("VariableToggle", "Variable Toggle"));

	UEditorGraph_Blueprint_Template* Blueprint = Editor.Pin()->GetTemplateBlueprintObj();

	const TArray<TWeakObjectPtr<UObject>>& Selections = PropertyView->GetSelectedObjects();
	for (TWeakObjectPtr<UObject> Obj : Selections)
	{
		if (UBP_GraphNode_Template* Node = Cast<UBP_GraphNode_Template>(Obj.Get()))
		{
			FName VarName = Node->GetFName();
			Node->bIsVariable = CheckState == ECheckBoxState::Checked;
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		}
	}

	PropertyView->ForceRefresh();
}

#undef LOCTEXT_NAMESPACE
