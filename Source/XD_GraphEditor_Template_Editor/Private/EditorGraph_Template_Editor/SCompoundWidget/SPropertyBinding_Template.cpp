// Fill out your copyright notice in the Description page of Project Settings.

#include "SPropertyBinding_Template.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "SComboButton.h"
#include "DetailWidgetRow.h"
#include "ObjectEditorUtils.h"
#include "KismetEditorUtilities.h"
#include "SWidgetSwitcher.h"
#include "IDetailPropertyExtensionHandler.h"
#include "BlueprintEditorUtils.h"
#include "EditorGraph_Blueprint_Template.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

void SPropertyBinding_Template::Construct(const FArguments& InArgs, FGraphEditor_Template* InEditor, UDelegateProperty* DelegateProperty, TSharedRef<IPropertyHandle> Property)
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
			.OnGetMenuContent(this, &SPropertyBinding_Template::OnGenerateDelegateMenu, Object, Property)
			.ContentPadding(1)
			.ButtonContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(this, &SPropertyBinding_Template::GetCurrentBindingImage, Property)
					.ColorAndOpacity(FLinearColor(0.25f, 0.25f, 0.25f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(4, 1, 0, 0)
				[
					SNew(STextBlock)
					.Text(this, &SPropertyBinding_Template::GetCurrentBindingText, Property)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
			.Visibility(this, &SPropertyBinding_Template::GetGotoBindingVisibility, Property)
			.OnClicked(this, &SPropertyBinding_Template::HandleGotoBindingClicked, Property)
			.VAlign(VAlign_Center)
			.ToolTipText(LOCTEXT("GotoFunction", "Goto Function"))
			[
				SNew(SImage)
				.Image(FEditorStyle::GetBrush("PropertyWindow.Button_Browse"))
			]
		]
	];
}

TSharedRef<SWidget> SPropertyBinding_Template::OnGenerateDelegateMenu(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle)
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
				FUIAction(FExecuteAction::CreateSP(this, &SPropertyBinding_Template::HandleRemoveBinding, PropertyHandle))
			);
		}
		else
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("CreateBinding", "添加绑定"),
				LOCTEXT("CreateBindingToolTip", "在当前的属性上创建函数绑定"),
				FSlateIcon(FEditorStyle::GetStyleSetName(), "Plus"),
				FUIAction(FExecuteAction::CreateSP(this, &SPropertyBinding_Template::HandleCreateAndAddBinding, Object, PropertyHandle))
			);
		}
	}
	MenuBuilder.EndSection(); //CreateBinding

	if (UClass* OwnerClass = Editor->GetBlueprintObj()->GeneratedClass)
	{
		TArray<UField*> BindingChain;
		bool bFoundEntry = false;

		static FName FunctionIcon(TEXT("GraphEditor.Function_16x"));

		MenuBuilder.BeginSection("Functions", LOCTEXT("Functions", "Functions"));
		{
			ForEachBindableFunction(OwnerClass, [&](TSharedPtr<FFunctionInfo> Info) {
				TArray<UField*> NewBindingChain(BindingChain);
				NewBindingChain.Add(Info->Function);

				bFoundEntry = true;

				MenuBuilder.AddMenuEntry(
					Info->DisplayName,
					FText::FromString(Info->Tooltip),
					FSlateIcon(FEditorStyle::GetStyleSetName(), FunctionIcon),
					FUIAction(FExecuteAction::CreateSP(this, &SPropertyBinding_Template::HandleAddFunctionBinding, PropertyHandle, Info, NewBindingChain))
				);
			});
		}
		MenuBuilder.EndSection(); //Functions

		if (bFoundEntry == false)
		{
			MenuBuilder.BeginSection("None", OwnerClass->GetDisplayNameText());
			MenuBuilder.AddWidget(SNew(STextBlock).Text(LOCTEXT("None", "None")), FText::GetEmpty());
			MenuBuilder.EndSection(); //None
		}
	}

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

const FSlateBrush* SPropertyBinding_Template::GetCurrentBindingImage(TSharedRef<IPropertyHandle> PropertyHandle) const
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

FText SPropertyBinding_Template::GetCurrentBindingText(TSharedRef<IPropertyHandle> PropertyHandle) const
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

				FGuid SearchForGuid = Binding.MemberFunctionGuid;

				for (UEdGraph* Graph : AllGraphs)
				{
					if (Graph->GraphGuid == SearchForGuid)
					{
						FName FoundName = Blueprint->GetFieldNameFromClassByGuid<UFunction>(Blueprint->GeneratedClass, Binding.MemberFunctionGuid);
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

EVisibility SPropertyBinding_Template::GetGotoBindingVisibility(TSharedRef<IPropertyHandle> PropertyHandle) const
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

				FGuid SearchForGuid = Binding.MemberFunctionGuid;

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

FReply SPropertyBinding_Template::HandleGotoBindingClicked(TSharedRef<IPropertyHandle> PropertyHandle)
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

				FGuid SearchForGuid = Binding.MemberFunctionGuid;

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

bool SPropertyBinding_Template::CanRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle) const
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

void SPropertyBinding_Template::HandleRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle)
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

void SPropertyBinding_Template::HandleCreateAndAddBinding(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle)
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
	Binding.MemberFunctionGuid = FunctionGraph->GraphGuid;
	Blueprint->Bindings.Add(Binding);

	GotoFunction(FunctionGraph);
}

void SPropertyBinding_Template::GotoFunction(UEdGraph* FunctionGraph)
{
	Editor->SetCurrentMode(FBlueprintApplicationModesTemplate::GraphMode);

	Editor->OpenDocument(FunctionGraph, FDocumentTracker::OpenNewDocument);
}

void SPropertyBinding_Template::HandleAddFunctionBinding(TSharedRef<IPropertyHandle> PropertyHandle, TSharedPtr<FFunctionInfo> SelectedFunction, TArray<UField*> BindingChain)
{
	const FScopedTransaction Transaction(LOCTEXT("BindDelegate", "Set Binding"));

	UEditorGraph_Blueprint_Template* Blueprint = Editor->GetTemplateBlueprintObj();
	Blueprint->Modify();

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);
	for (UObject* SelectedObject : OuterObjects)
	{
		FDelegateEditorBinding_Template Binding;
		Binding.Object = SelectedObject;
		Binding.PropertyName = PropertyHandle->GetProperty()->GetFName();
		Blueprint->Bindings.Add(Binding);

		if (SelectedFunction->Function)
		{
			UBlueprint::GetGuidFromClassByFieldName<UFunction>(
				SelectedFunction->Function->GetOwnerClass(),
				SelectedFunction->Function->GetFName(),
				Binding.MemberFunctionGuid);
		}

		Blueprint->Bindings.RemoveAll([&](const FDelegateEditorBinding_Template& E) {return E.Object == Binding.Object && E.PropertyName == Binding.PropertyName; });
		Blueprint->Bindings.Add(Binding);
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
}

bool FDetailExtensionHandler_Template::IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& InPropertyHandle) const
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

TSharedRef<SWidget> FDetailExtensionHandler_Template::GenerateExtensionWidget(const UClass* InObjectClass, TSharedPtr<IPropertyHandle> InPropertyHandle)
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

	return SNew(SPropertyBinding_Template, BlueprintEditor, DelegateProperty, InPropertyHandle.ToSharedRef());
}

template <typename Predicate>
void SPropertyBinding_Template::ForEachBindableFunction(UClass* FromClass, Predicate Pred) const
{
	const FSlateFontInfo DetailFontInfo = IDetailLayoutBuilder::GetDetailFont();

	UEditorGraph_Blueprint_Template* Blueprint = Editor->GetTemplateBlueprintObj();
	UBlueprintGeneratedClass* SkeletonClass = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);

	// Walk up class hierarchy for native functions and properties
	for (TFieldIterator<UFunction> FuncIt(FromClass, EFieldIteratorFlags::IncludeSuper); FuncIt; ++FuncIt)
	{
		UFunction* Function = *FuncIt;

		// Only allow binding pure functions if we're limited to pure function bindings.
		if (GeneratePureBindings && !Function->HasAnyFunctionFlags(FUNC_Const | FUNC_BlueprintPure))
		{
			continue;
		}

		// C++中定义的函数跳过
		if (Function->HasAnyFunctionFlags(FUNC_Native))
		{
			continue;
		}

		// Only bind to functions that are callable from blueprints
		if (!UEdGraphSchema_K2::CanUserKismetCallFunction(Function))
		{
			continue;
		}

		// We ignore CPF_ReturnParm because all that matters for binding to script functions is that the number of out parameters match.
		if (Function->IsSignatureCompatibleWith(BindableSignature, UFunction::GetDefaultIgnoredSignatureCompatibilityFlags() | CPF_ReturnParm))
		{
			TSharedPtr<FFunctionInfo> Info = MakeShareable(new FFunctionInfo());
			Info->DisplayName = FText::FromName(Function->GetFName());
			Info->Tooltip = Function->GetMetaData("Tooltip");
			Info->FuncName = Function->GetFName();
			Info->Function = Function;

			Pred(Info);
		}
	}
}

#undef LOCTEXT_NAMESPACE
