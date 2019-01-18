// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintApplicationModeTemplate.h"
#include "SModeWidget.h"
#include "IDocumentation.h"

#define LOCTEXT_NAMESPACE "GraphEditor_Template"

const FName FBlueprintApplicationModesTemplate::DesignerMode("DesignerName");
const FName FBlueprintApplicationModesTemplate::GraphMode("GraphName");

FText FBlueprintApplicationModesTemplate::GetLocalizedMode(const FName InMode)
{
	static TMap< FName, FText > LocModes;

	if (LocModes.Num() == 0)
	{
		LocModes.Add(DesignerMode, LOCTEXT("DesignerMode", "设计"));
		LocModes.Add(GraphMode, LOCTEXT("GraphMode", "蓝图"));
	}

	check(InMode != NAME_None);
	const FText* OutDesc = LocModes.Find(InMode);
	check(OutDesc);

	return *OutDesc;
}

FBlueprintApplicationModeTemplate::FBlueprintApplicationModeTemplate(TSharedPtr<class FGraphEditor_Template> GraphEditorToolkit, FName InModeName)
	: FBlueprintEditorApplicationMode(GraphEditorToolkit, InModeName, FBlueprintApplicationModesTemplate::GetLocalizedMode, false, false)
	, MyEditor_Template(GraphEditorToolkit)
{
}

void FBlueprintApplicationModeTemplate::AddModeSwitchToolBarExtension()
{
	FGraphEditor_Template* GraphEditor = MyEditor_Template.Pin().Get();

	ToolbarExtender->AddToolBarExtension("Asset",
		EExtensionHook::After,
		GraphEditor->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([=](FToolBarBuilder& ToolbarBuilder)
	{

		TAttribute<FName> GetActiveMode;
		GetActiveMode.BindRaw(GraphEditor, &FBlueprintEditor::GetCurrentMode);
		FOnModeChangeRequested SetActiveMode = FOnModeChangeRequested::CreateRaw(GraphEditor, &FBlueprintEditor::SetCurrentMode);

		// Left side padding
		GraphEditor->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));

		GraphEditor->AddToolbarWidget(
			SNew(SModeWidget, FBlueprintApplicationModesTemplate::GetLocalizedMode(FBlueprintApplicationModesTemplate::DesignerMode), FBlueprintApplicationModesTemplate::DesignerMode)
			.OnGetActiveMode(GetActiveMode)
			.OnSetActiveMode(SetActiveMode)
			.ToolTip(IDocumentation::Get()->CreateToolTip(
				LOCTEXT("DesignerModeButtonTooltip", "Switch to Blueprint Designer Mode"),
				NULL,
				TEXT("Shared/Editors/BlueprintEditor"),
				TEXT("DesignerMode")))
			.IconImage(FEditorStyle::GetBrush("UMGEditor.SwitchToDesigner"))
			.SmallIconImage(FEditorStyle::GetBrush("UMGEditor.SwitchToDesigner.Small"))
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("DesignerMode")))
		);

		class SBlueprintModeSeparator : public SBorder
		{
		public:
			SLATE_BEGIN_ARGS(SBlueprintModeSeparator) {}
			SLATE_END_ARGS()

				void Construct(const FArguments& InArg)
			{
				SBorder::Construct(
					SBorder::FArguments()
					.BorderImage(FEditorStyle::GetBrush("BlueprintEditor.PipelineSeparator"))
					.Padding(0.0f)
				);
			}

			// SWidget interface
			virtual FVector2D ComputeDesiredSize(float) const override
			{
				const float Height = 20.0f;
				const float Thickness = 16.0f;
				return FVector2D(Thickness, Height);
			}
			// End of SWidget interface
		};

		GraphEditor->AddToolbarWidget(SNew(SBlueprintModeSeparator));

		GraphEditor->AddToolbarWidget(
			SNew(SModeWidget, FBlueprintApplicationModesTemplate::GetLocalizedMode(FBlueprintApplicationModesTemplate::GraphMode), FBlueprintApplicationModesTemplate::GraphMode)
			.OnGetActiveMode(GetActiveMode)
			.OnSetActiveMode(SetActiveMode)
			//.CanBeSelected(GraphEditor.Get(), &FBlueprintEditor::IsEditingSingleBlueprint)
			.ToolTip(IDocumentation::Get()->CreateToolTip(
				LOCTEXT("GraphModeButtonTooltip", "Switch to Graph Editing Mode"),
				NULL,
				TEXT("Shared/Editors/BlueprintEditor"),
				TEXT("GraphMode")))
			.ToolTipText(LOCTEXT("GraphModeButtonTooltip", "Switch to Graph Editing Mode"))
			.IconImage(FEditorStyle::GetBrush("FullBlueprintEditor.SwitchToScriptingMode"))
			.SmallIconImage(FEditorStyle::GetBrush("FullBlueprintEditor.SwitchToScriptingMode.Small"))
			.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("GraphMode")))
		);

		// Right side padding
		GraphEditor->AddToolbarWidget(SNew(SSpacer).Size(FVector2D(4.0f, 1.0f)));
	}));
}

UEditorGraph_Blueprint_Template* FBlueprintApplicationModeTemplate::GetBlueprint() const
{
	if (FGraphEditor_Template* Editor = MyEditor_Template.Pin().Get())
	{
		return Editor->GetTemplateBlueprintObj();
	}
	else
	{
		return NULL;
	}
}

class FGraphEditor_Template* FBlueprintApplicationModeTemplate::GetBlueprintEditor() const
{
	return MyEditor_Template.Pin().Get();
}

#undef LOCTEXT_NAMESPACE
