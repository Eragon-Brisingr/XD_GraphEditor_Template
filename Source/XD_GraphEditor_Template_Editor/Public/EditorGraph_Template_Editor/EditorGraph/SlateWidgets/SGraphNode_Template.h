// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "IDetailsView.h"

/**
 * 
 */
class SGraphNode_Template : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_Template) {}
	SLATE_END_ARGS()

	// Inherited via SGraphNode
	void Construct(const FArguments& InArgs, UEdGraphNode* InNode);
	void UpdateGraphNode() override;
	void CreatePinWidgets() override;
	void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	bool IsNameReadOnly() const override;
	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);

protected:

	TSharedPtr<SBox> ContentWidget;
    TSharedPtr<STextBlock> NodeHeader;

	virtual void CreateContent();
    virtual void CreateHeader();
};
