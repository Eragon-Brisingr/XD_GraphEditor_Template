// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/EditorGraph/SlateWidgets/SGraphPin_Template.h"

#include "EditorGraph_Template_Editor/Utility/GraphEditorStyle_Template.h"


void SGraphPin_Template::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	this->SetCursor(EMouseCursor::Default);

	bShowLabel = true;

	GraphPinObj = InPin;
	check(GraphPinObj != nullptr);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &SGraphPin_Template::GetPinBorder)
		.BorderBackgroundColor(this, &SGraphPin_Template::GetPinColor)
		.OnMouseButtonDown(this, &SGraphPin_Template::OnPinMouseDown)
		.Cursor(this, &SGraphPin_Template::GetPinCursor)
		.Padding(FMargin(10.0f))
	);
}

FSlateColor SGraphPin_Template::GetPinColor() const
{
	return GraphEditorStyle_Template::Pin::Default;
}

TSharedRef<SWidget> SGraphPin_Template::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

const FSlateBrush* SGraphPin_Template::GetPinBorder() const
{
	return FEditorStyle::GetBrush(TEXT("Graph.StateNode.Body"));
}
