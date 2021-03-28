// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailPropertyExtensionHandler.h"
#include "Widgets/SCompoundWidget.h"
#include "UObject/UnrealTypePrivate.h"

class IDetailLayoutBuilder;
class FGraphEditor_Template;
class IPropertyHandle;
struct FFunctionInfo;

/**
 * 
 */
class XD_GRAPHEDITOR_TEMPLATE_EDITOR_API SPropertyBinding_Template : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SPropertyBinding_Template)
	{}
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, FGraphEditor_Template* InEditor, FDelegateProperty* DelegateProperty, TSharedRef<IPropertyHandle> Property);

	TSharedRef<SWidget> OnGenerateDelegateMenu(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle);

	const FSlateBrush* GetCurrentBindingImage(TSharedRef<IPropertyHandle> PropertyHandle) const;

	FText GetCurrentBindingText(TSharedRef<IPropertyHandle> PropertyHandle) const;

	EVisibility GetGotoBindingVisibility(TSharedRef<IPropertyHandle> PropertyHandle) const;

	FReply HandleGotoBindingClicked(TSharedRef<IPropertyHandle> PropertyHandle);

	bool CanRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle) const;

	void HandleRemoveBinding(TSharedRef<IPropertyHandle> PropertyHandle);

	void HandleCreateAndAddBinding(UObject* Object, TSharedRef<IPropertyHandle> PropertyHandle);

	void GotoFunction(UEdGraph* FunctionGraph);

	void HandleAddFunctionBinding(TSharedRef<IPropertyHandle> PropertyHandle, TSharedPtr<FFunctionInfo> SelectedFunction, TArray<UField*> BindingChain);
private:
	template <typename Predicate>
	void ForEachBindableFunction(UClass* FromClass, Predicate Pred) const;

	FGraphEditor_Template* Editor;

	bool GeneratePureBindings = true;
	UFunction* BindableSignature;
};

class FDetailExtensionHandler_Template : public IDetailPropertyExtensionHandler
{
public:
	FDetailExtensionHandler_Template(FGraphEditor_Template* BlueprintEditor)
		: BlueprintEditor(BlueprintEditor)
	{}

	bool IsPropertyExtendable(const UClass* InObjectClass, const IPropertyHandle& InPropertyHandle) const override;
	TSharedRef<SWidget> GenerateExtensionWidget(const IDetailLayoutBuilder& InDetailBuilder, const UClass* InObjectClass, TSharedPtr<IPropertyHandle> InPropertyHandle) override;

private:
	FGraphEditor_Template* BlueprintEditor;
};