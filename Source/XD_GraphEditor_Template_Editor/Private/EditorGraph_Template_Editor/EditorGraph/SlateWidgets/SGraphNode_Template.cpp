// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/EditorGraph/SlateWidgets/SGraphNode_Template.h"
#include <Slate.h>
#include <GraphEditorSettings.h>
#include <SGraphPin.h>
#include <SCommentBubble.h>

#include "EditorGraph_Template_Editor/EditorGraph/EditorNodes/Editor_GraphNode_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/SlateWidgets/SGraphPin_Template.h"
#include "EditorGraph_Template_Editor/Utility/GraphEditor_Template_Log.h"

#define LOCTEXT_NAMESPACE "SBP_Graph_TemplateNode"

class UEditor_GraphNode_Template;

void SGraphNode_Template::Construct(const FArguments & InArgs, UEdGraphNode * InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGraphNode_Template::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);
	TSharedPtr<SErrorText> ErrorText;

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(FMargin(1.f, 5.0f))
			.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.1f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(LeftNodeBox, SVerticalBox)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				[
					SNew(SBorder)
                    .BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
                    .Padding(FMargin(5.0f))
                    .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.4f))
                    [
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.AutoHeight()
						[
							SAssignNew(NodeHeader, STextBlock)
						]
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.AutoHeight()
						[
							SAssignNew(InlineEditableText, SInlineEditableTextBlock)
							.Style(FEditorStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
							.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
							.IsReadOnly(this, &SGraphNode_Template::IsNameReadOnly)
							.OnTextCommitted(this, &SGraphNode_Template::OnNameTextCommited)
							.OnVerifyTextChanged(this, &SGraphNode_Template::OnVerifyNameTextChanged)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							NodeTitle.ToSharedRef()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SAssignNew(ContentWidget, SBox)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							// 错误提示
							SAssignNew(ErrorText, SErrorText)
							.BackgroundColor(this, &SGraphNode_Template::GetErrorColor)
							.ToolTipText(this, &SGraphNode_Template::GetErrorMsgToolTip)
						]
                    ]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(RightNodeBox, SVerticalBox)
				]
			]
		];

	//注释用的气泡
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	UpdateErrorInfo();
	ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);

	CreatePinWidgets();
	CreateContent();
    CreateHeader();
}

void SGraphNode_Template::CreatePinWidgets()
{
	UEditor_GraphNode_Template* EdNode = CastChecked<UEditor_GraphNode_Template>(GraphNode);
	for (int32 i = 0; i < EdNode->Pins.Num(); ++i)
	{
		UEdGraphPin* Pin = EdNode->Pins[i];
		if (!Pin->bHidden)
		{
			TSharedPtr<SGraphPin>NewPin = SNew(SGraphPin_Template, Pin);
			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SGraphNode_Template::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(0.f, 0.0f)
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
	else
	{
		RightNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			.Padding(0.f, 0.0f)
			[
				PinToAdd
			];
		OutputPins.Add(PinToAdd);
	}
}

bool SGraphNode_Template::IsNameReadOnly() const
{
	return false;
}

void SGraphNode_Template::OnNameTextCommited(const FText & InText, ETextCommit::Type CommitInfo)
{
	UEditor_GraphNode_Template* UEdNode = CastChecked<UEditor_GraphNode_Template>(GraphNode);

	if (UEdNode)
		if (UEdNode->RenameUniqueNode(InText))
		{
			UpdateGraphNode();
            NodeHeader.Get()->SetVisibility(EVisibility::Visible);
			SGraphNode::OnNameTextCommited(InText, CommitInfo);
		}

}

void SGraphNode_Template::CreateContent()
{
	UEditor_GraphNode_Template* Node = Cast<UEditor_GraphNode_Template>(GraphNode);

	ContentWidget->SetContent(Node->GetContentWidget().ToSharedRef());
	ContentWidget->SetMinDesiredWidth(200.f);
}

void SGraphNode_Template::CreateHeader()
{
    NodeHeader.Get()->SetText(GraphNode->GetNodeTitle(ENodeTitleType::MenuTitle));

    UEditor_GraphNode_Template* UEdNode = CastChecked<UEditor_GraphNode_Template>(GraphNode);

    if (UEdNode)
    {
		NodeHeader.Get()->SetVisibility((UEdNode->GetEdNodeName().IsEmpty()) ? EVisibility::Collapsed : EVisibility::Visible);
    }
    else
    {
		GraphEditor_Template_Error_Log("An error occurred when creating the slate node headers");
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE