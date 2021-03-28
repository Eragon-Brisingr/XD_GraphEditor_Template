// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Misc/NotifyHook.h"

class FGraphEditor_Template;

/**
 * 
 */
class XD_GRAPHEDITOR_TEMPLATE_EDITOR_API SDetailsView_Template : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SDetailsView_Template) {}
	SLATE_END_ARGS()
public:
	TSharedPtr<SEditableTextBox> NameTextBox;
	TSharedPtr<class IDetailsView> PropertyView;

	TWeakPtr<FGraphEditor_Template> Editor;

	void Construct(const FArguments& InArgs, TWeakPtr<FGraphEditor_Template> InEditor);

	EVisibility GetNameAreaVisibility() const;

	const FSlateBrush* GetNameIcon() const;

	FText GetNameText() const;

	bool HandleVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage);

	void HandleNameTextChanged(const FText& Text);

	void HandleNameTextCommitted(const FText& Text, ETextCommit::Type CommitType);

	ECheckBoxState GetIsVariable() const;

	void HandleIsVariableChanged(ECheckBoxState CheckState);
};
