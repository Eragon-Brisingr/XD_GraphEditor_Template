// Copyright (c) 2018 Jhonny Hueller
#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"

#define LOCTEXT_NAMESPACE "SEditorGraph_TemplateGraphPin"

/**
 * 
 */
class SGraphPin_Template : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SGraphPin_Template) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

protected:
	virtual FSlateColor GetPinColor() const override;

	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;

	const FSlateBrush* GetPinBorder() const;
};

#undef LOCTEXT_NAMESPACE