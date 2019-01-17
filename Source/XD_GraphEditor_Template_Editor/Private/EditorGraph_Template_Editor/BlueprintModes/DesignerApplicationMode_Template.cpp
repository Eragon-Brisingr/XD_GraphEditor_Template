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
#include "IDetailPropertyExtensionHandler.h"
#include "MultiBoxBuilder.h"
#include "SlateApplication.h"
#include "ScopedTransaction.h"
#include "EditorGraph_Blueprint_Template.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

const FName FDesignerApplicationMode_Template::DetailsTabId(TEXT("Designer_Template_DetailsTabId"));
const FName FDesignerApplicationMode_Template::GraphTabId(TEXT("Designer_Template_GraphTabId"));

struct FFunctionInfo
{
	FFunctionInfo()
		: Function(nullptr)
	{
	}

	FText DisplayName;
	FString Tooltip;

	FName FuncName;
	UFunction* Function;
};

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

		class SPropertyBinding : public SCompoundWidget
		{
		public:

			SLATE_BEGIN_ARGS(SPropertyBinding)
			{}
			SLATE_END_ARGS()

		public:
			void Construct(const FArguments& InArgs, FGraphEditorToolkit_Template* InEditor, UDelegateProperty* DelegateProperty, TSharedRef<IPropertyHandle> Property)
			{
				Editor = InEditor;
				BindableSignature = DelegateProperty->SignatureFunction;

				TArray<UObject*> Objects;
				Property->GetOuterObjects(Objects);
				UObject* Object = Objects[0];

				ChildSlot
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SComboButton)
						.OnGetMenuContent(this, &SPropertyBinding::OnGenerateDelegateMenu, Object, Property)
						.ContentPadding(1)
						.ButtonContent()
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SImage)
								.Image(this, &SPropertyBinding::GetCurrentBindingImage, Property)
								.ColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.25f))
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							.Padding(4, 1, 0, 0)
							[
								SNew(STextBlock)
								.Text(this, &SPropertyBinding::GetCurrentBindingText, Property)
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
						.Visibility(this, &SPropertyBinding::GetGotoBindingVisibility, Property)
						.OnClicked(this, &SPropertyBinding::HandleGotoBindingClicked, Property)
						.VAlign(VAlign_Center)
						.ToolTipText(LOCTEXT("GotoFunction", "Goto Function"))
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Browse"))
						]
					]
				];
			}

			TSharedRef<SWidget> OnGenerateDelegateMenu(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle)
			{
				const bool bInShouldCloseWindowAfterMenuSelection = true;
				FMenuBuilder MenuBuilder(bInShouldCloseWindowAfterMenuSelection, nullptr);

				MenuBuilder.BeginSection("BindingActions");
				{
					if (CanRemoveBinding(PropertyHandle))
					{
						MenuBuilder.AddMenuEntry(
							LOCTEXT("RemoveBinding", "移除绑定"),
							LOCTEXT("RemoveBindingToolTip", "移除当前绑定"),
							FSlateIcon(FEditorStyle::GetStyleSetName(), "Cross"),
							FUIAction(FExecuteAction::CreateSP(this, &SPropertyBinding::HandleRemoveBinding, PropertyHandle))
						);
					}
					else
					{
						MenuBuilder.AddMenuEntry(
							LOCTEXT("CreateBinding", "添加绑定"),
							LOCTEXT("CreateBindingToolTip", "在当前的属性上创建函数绑定"),
							FSlateIcon(FEditorStyle::GetStyleSetName(), "Plus"),
							FUIAction(FExecuteAction::CreateSP(this, &SPropertyBinding::HandleCreateAndAddBinding, Object, PropertyHandle))
						);
					}
				}
				MenuBuilder.EndSection(); //CreateBinding

				FDisplayMetrics DisplayMetrics;
				FSlateApplication::Get().GetCachedDisplayMetrics(DisplayMetrics);

				return
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.MaxHeight(DisplayMetrics.PrimaryDisplayHeight * 0.5)
					[
						MenuBuilder.MakeWidget()
					];
			}

			const FSlateBrush* GetCurrentBindingImage(TSharedRef<IPropertyHandle> PropertyHandle) const
			{
				static FName PropertyIcon(TEXT("Kismet.Tabs.Variables"));
				static FName FunctionIcon(TEXT("GraphEditor.Function_16x"));

				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);

				//TODO UMG O(N) Isn't good for this, needs to be map, but map isn't serialized, need cached runtime map for fast lookups.

				FName PropertyName = PropertyHandle->GetProperty()->GetFName();
				for (int32 ObjectIndex = 0; ObjectIndex < OuterObjects.Num(); ObjectIndex++)
				{
					// Ignore null outer objects
					if (OuterObjects[ObjectIndex] == NULL)
					{
						continue;
					}

					if (!CanRemoveBinding(PropertyHandle))
					{
						return FEditorStyle::GetBrush(FunctionIcon);
					}
				}

				return nullptr;
			}

			FText GetCurrentBindingText(TSharedRef<IPropertyHandle> PropertyHandle) const
			{
				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);

				FName PropertyName = PropertyHandle->GetProperty()->GetFName();
				for (int32 ObjectIndex = 0; ObjectIndex < OuterObjects.Num(); ObjectIndex++)
				{
					// Ignore null outer objects
					if (OuterObjects[ObjectIndex] == nullptr)
					{
						continue;
					}

					UEditorGraph_Blueprint_Template* Blueprint = Editor->GetTemplateBlueprintObj();

					for (const FDelegateEditorBinding_Template& Binding : Blueprint->Bindings)
					{
						if (Binding.Object.Get() == OuterObjects[ObjectIndex] && Binding.PropertyName == PropertyName)
						{
							TArray<UEdGraph*> AllGraphs;
							Blueprint->GetAllGraphs(AllGraphs);

							FGuid SearchForGuid = Binding.MemberGuid;

							for (UEdGraph* Graph : AllGraphs)
							{
								if (Graph->GraphGuid == SearchForGuid)
								{
									FName FoundName = Blueprint->GetFieldNameFromClassByGuid<UFunction>(Blueprint->GeneratedClass, Binding.MemberGuid);
									return FText::FromString(FName::NameToDisplayString(FoundName.ToString(), false));
								}
							}

							return LOCTEXT("绑定函数丢失", "绑定函数丢失");
						}
					}

					break;
				}

				return LOCTEXT("未绑定", "未绑定");
			}

			EVisibility GetGotoBindingVisibility(TSharedRef<IPropertyHandle> PropertyHandle) const
			{
				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);

				FName PropertyName = PropertyHandle->GetProperty()->GetFName();
				for (int32 ObjectIndex = 0; ObjectIndex < OuterObjects.Num(); ObjectIndex++)
				{
					// Ignore null outer objects
					if (OuterObjects[ObjectIndex] == nullptr)
					{
						continue;
					}

					UEditorGraph_Blueprint_Template* Blueprint = Editor->GetTemplateBlueprintObj();

					for (const FDelegateEditorBinding_Template& Binding : Blueprint->Bindings)
					{
						if (Binding.Object.Get() == OuterObjects[ObjectIndex])
						{
							TArray<UEdGraph*> AllGraphs;
							Blueprint->GetAllGraphs(AllGraphs);

							FGuid SearchForGuid = Binding.MemberGuid;

							for (UEdGraph* Graph : AllGraphs)
							{
								if (Graph->GraphGuid == SearchForGuid)
								{
									return EVisibility::Visible;
								}
							}
						}
					}

				}

				return EVisibility::Collapsed;
			}

			FReply HandleGotoBindingClicked(TSharedRef<IPropertyHandle> PropertyHandle)
			{
				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);

				FName PropertyName = PropertyHandle->GetProperty()->GetFName();
				for (int32 ObjectIndex = 0; ObjectIndex < OuterObjects.Num(); ObjectIndex++)
				{
					// Ignore null outer objects
					if (OuterObjects[ObjectIndex] == nullptr)
					{
						continue;
					}

					UEditorGraph_Blueprint_Template* Blueprint = Editor->GetTemplateBlueprintObj();

 					for (const FDelegateEditorBinding_Template& Binding : Blueprint->Bindings)
 					{
						if (Binding.Object.Get() == OuterObjects[ObjectIndex] && Binding.PropertyName == PropertyName)
						{
							TArray<UEdGraph*> AllGraphs;
							Blueprint->GetAllGraphs(AllGraphs);

							FGuid SearchForGuid = Binding.MemberGuid;

							for (UEdGraph* Graph : AllGraphs)
							{
								if (Graph->GraphGuid == SearchForGuid)
								{
									GotoFunction(Graph);
									return FReply::Handled();
								}
							}
						}
 					}
				}

				return FReply::Unhandled();
			}

			bool CanRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle) const
			{
				FName PropertyName = PropertyHandle->GetProperty()->GetFName();

				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);
				for (UObject* SelectedObject : OuterObjects)
				{
					UEditorGraph_Blueprint_Template* Blueprint = Editor->GetTemplateBlueprintObj();

					for (const FDelegateEditorBinding_Template& Binding : Blueprint->Bindings)
					{
						if (Binding.Object.Get() == SelectedObject && Binding.PropertyName == PropertyName)
						{
							return true;
						}
					}
				}

				return false;
			}

			void HandleRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle)
			{
				UEditorGraph_Blueprint_Template* Blueprint = Editor->GetTemplateBlueprintObj();

				const FScopedTransaction Transaction(LOCTEXT("UnbindDelegate", "Remove Binding"));

				Blueprint->Modify();

				TArray<UObject*> OuterObjects;
				PropertyHandle->GetOuterObjects(OuterObjects);
				for (UObject* SelectedObject : OuterObjects)
				{
					Blueprint->Bindings.RemoveAll([&](const FDelegateEditorBinding_Template& Binding) {return Binding.Object == SelectedObject && Binding.PropertyName == PropertyHandle->GetProperty()->GetFName(); });
				}			

				FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
			}

			void HandleCreateAndAddBinding(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle)
			{
				UEditorGraph_Blueprint_Template* Blueprint = Editor->GetTemplateBlueprintObj();

				const FScopedTransaction Transaction(LOCTEXT("CreateDelegate", "Create Binding"));

				Blueprint->Modify();

				FString Pre = GeneratePureBindings ? FString(TEXT("Get")) : FString(TEXT("On"));

 				FString	ObjectName = TEXT("_") + Object->GetName() + TEXT("_");

				FString Post = PropertyHandle->GetProperty()->GetName();
				Post.RemoveFromStart(TEXT("On"));
				Post.RemoveFromEnd(TEXT("Event"));

				// Create the function graph.
				FString FunctionName = Pre + ObjectName + Post;
				UEdGraph* FunctionGraph = FBlueprintEditorUtils::CreateNewGraph(
					Blueprint,
					FBlueprintEditorUtils::FindUniqueKismetName(Blueprint, FunctionName),
					UEdGraph::StaticClass(),
					UEdGraphSchema_K2::StaticClass());

				// Add the binding to the blueprint
				TSharedPtr<FFunctionInfo> SelectedFunction = MakeShareable(new FFunctionInfo());
				SelectedFunction->FuncName = FunctionGraph->GetFName();

				const bool bUserCreated = true;
				FBlueprintEditorUtils::AddFunctionGraph(Blueprint, FunctionGraph, bUserCreated, BindableSignature);

				// Only mark bindings as pure that need to be.
				if (GeneratePureBindings)
				{
					const UEdGraphSchema_K2* Schema_K2 = Cast<UEdGraphSchema_K2>(FunctionGraph->GetSchema());
					Schema_K2->AddExtraFunctionFlags(FunctionGraph, FUNC_BlueprintPure);
				}

				FDelegateEditorBinding_Template Binding;
				Binding.Object = Object;
				Binding.PropertyName = PropertyHandle->GetProperty()->GetFName();
				Binding.MemberGuid = FunctionGraph->GraphGuid;
				Blueprint->Bindings.Add(Binding);

				GotoFunction(FunctionGraph);
			}

			void GotoFunction(UEdGraph* FunctionGraph)
			{
				Editor->SetCurrentMode(FBlueprintApplicationModesTemplate::GraphMode);

				Editor->OpenDocument(FunctionGraph, FDocumentTracker::OpenNewDocument);
			}

		private:
			FGraphEditorToolkit_Template* Editor;

			bool GeneratePureBindings = true;
			UFunction* BindableSignature;
		};

		class FDetail_TemplateExtensionHandler : public IDetailPropertyExtensionHandler
		{
		public:
			FDetail_TemplateExtensionHandler(FGraphEditorToolkit_Template* BlueprintEditor)
				: BlueprintEditor(BlueprintEditor)
			{}

			virtual bool IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& InPropertyHandle) const override
			{
				if (InPropertyHandle.GetNumOuterObjects() == 1)
				{
					TArray<UObject*> Objects;
					InPropertyHandle.GetOuterObjects(Objects);

					// We don't allow bindings on the CDO.
					if (Objects[0] != nullptr && Objects[0]->HasAnyFlags(RF_ClassDefaultObject))
					{
						return false;
					}

					UProperty* Property = InPropertyHandle.GetProperty();
					FString DelegateName = Property->GetName() + "Delegate";

					if (UClass* ContainerClass = Cast<UClass>(Property->GetOuter()))
					{
						UDelegateProperty* DelegateProperty = FindField<UDelegateProperty>(ContainerClass, FName(*DelegateName));
						if (DelegateProperty)
						{
							return true;
						}
					}
				}

				return false;
			}

			virtual TSharedRef<SWidget> GenerateExtensionWidget(const UClass* InObjectClass, TSharedPtr<IPropertyHandle> InPropertyHandle) override
			{
				UProperty* Property = InPropertyHandle->GetProperty();
				FString DelegateName = Property->GetName() + "Delegate";

				UDelegateProperty* DelegateProperty = FindFieldChecked<UDelegateProperty>(CastChecked<UClass>(Property->GetOuter()), FName(*DelegateName));

				const bool bIsEditable = Property->HasAnyPropertyFlags(CPF_Edit | CPF_EditConst);
				const bool bDoSignaturesMatch = DelegateProperty->SignatureFunction->GetReturnProperty()->SameType(Property);

				if (!ensure(bIsEditable && bDoSignaturesMatch))
				{
					return SNullWidget::NullWidget;
				}

				return SNew(SPropertyBinding, BlueprintEditor, DelegateProperty, InPropertyHandle.ToSharedRef());
			}

		private:
			FGraphEditorToolkit_Template* BlueprintEditor;
		};

		PropertyView->SetExtensionHandler(MakeShareable(new FDetail_TemplateExtensionHandler(Editor.Pin().Get())));

		class FDesignerDelegate_Template : public IDetailCustomization
		{
			FDesignerDelegate_Template(FGraphEditorToolkit_Template* InEditor, UEditorGraph_Blueprint_Template* Blueprint)
				:Editor(InEditor), Blueprint(Blueprint)
			{}

			FGraphEditorToolkit_Template* Editor;
			UEditorGraph_Blueprint_Template* Blueprint;

			void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override
			{
				TArray< TWeakObjectPtr<UObject> > OutObjects;
				DetailLayout.GetObjectsBeingCustomized(OutObjects);

				if (OutObjects.Num() == 1)
				{
					UObject* Obj = OutObjects[0].Get();
					UClass* PropertyClass = Obj->GetClass();

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

				UObjectProperty* ComponentProperty = FindField<UObjectProperty>(Blueprint->SkeletonGeneratedClass, ThisComponentName);

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
			static TSharedRef<IDetailCustomization> MakeInstance(FGraphEditorToolkit_Template* Editor, UEditorGraph_Blueprint_Template* Blueprint)
			{
				return MakeShareable(new FDesignerDelegate_Template(Editor, Blueprint));
			}
		};

		FGraphEditorToolkit_Template* BlueprintEditor = Editor.Pin().Get();
		PropertyView->RegisterInstancedCustomPropertyLayout(UObject::StaticClass(), FOnGetDetailCustomizationInstance::CreateStatic(&FDesignerDelegate_Template::MakeInstance, BlueprintEditor, BlueprintEditor->GetTemplateBlueprintObj()));

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