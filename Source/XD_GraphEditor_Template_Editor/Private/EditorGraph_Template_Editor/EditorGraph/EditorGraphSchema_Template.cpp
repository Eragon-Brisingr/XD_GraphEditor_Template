// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template_Editor/EditorGraph/EditorGraphSchema_Template.h"
#include <EdGraph/EdGraph.h>
#include <UObject/UObjectIterator.h>
#include <Modules/ModuleManager.h>
#include <Templates/SubclassOf.h>

#include "EditorGraph_Template_Editor/Utility/GraphEditor_Template_Log.h"
#include "EditorGraph_Template_Editor/EditorGraph/FEditorConnectionDrawingPolicy_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/SchemaActions/FEditorGraph_NewNode_SchemaAction_Template.h"
#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"
#include "EditorGraph_Template_Editor/EditorGraph/EditorNodes/Editor_GraphNode_Template.h"
#include "EditorGraph_Template_Editor/Utility/GraphEditor_ClassHelper_Template.h"
#include "XD_GraphEditor_Template_Editor.h"

#define LOCTEXT_NAMESPACE "EditorGraphSchema_Template"

UEditorGraphSchema_Template::UEditorGraphSchema_Template(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer){}

 void UEditorGraphSchema_Template::GetGraphContextActions(FGraphContextMenuBuilder & ContextMenuBuilder) const
 {
 
 	FText ToolTip = LOCTEXT("NewEditorGraph_TemplateNodeTooltip", "Add a {NodeName} to the graph.");
 	FText MenuDesc = LOCTEXT("NewEditorGraph_TemplateNodeDescription", "{NodeName}");
 
 	FXD_GraphEditor_Template_EditorModule& Module = FModuleManager::LoadModuleChecked<FXD_GraphEditor_Template_EditorModule>("XD_GraphEditor_Template_Editor");
     TSharedPtr<FGraphEditor_ClassHelper_Template> Helper = Module.GetHelper();
 	
 	//Gathering C++ classes
 
 	FCategorizedGraphActionListBuilder BaseBuilder(TEXT("C++ Defined Nodes"));
 
     TArray<FGraphEditor_ClassData_Template> AllSubClasses;
     Helper->GatherClasses(UBP_GraphNode_Template::StaticClass(),AllSubClasses);
 
     for (auto& NativeClassData : AllSubClasses)
     {
         if (NativeClassData.GetClass()->HasAnyClassFlags(CLASS_Native))
         {
             FFormatNamedArguments Arguments;
             Arguments.Add(TEXT("NodeName"), NativeClassData.GetClass()->GetDisplayNameText());
             TSharedPtr<FEditorGraph_NewNode_SchemaAction_Template> NewNodeAction;
 
             NewNodeAction = MakeShareable(new FEditorGraph_NewNode_SchemaAction_Template(NativeClassData.GetCategory(), FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments), 0, NativeClassData.GetClass()));
 
             BaseBuilder.AddAction(NewNodeAction);
         }
     }
 
 	ContextMenuBuilder.Append(BaseBuilder);
 	
 	//Gathering child blueprints
     FCategorizedGraphActionListBuilder BlueprintBuilder(TEXT("Blueprint Defined Nodes"));
 
 	for (auto& BlueprintClassData : AllSubClasses)
 	{
 		if (!BlueprintClassData.GetClass()->HasAnyClassFlags(CLASS_Native))
 		{
 			FFormatNamedArguments Arguments;
 			Arguments.Add(TEXT("NodeName"), BlueprintClassData.GetClass()->GetDisplayNameText());
             TSharedPtr<FEditorGraph_NewNode_SchemaAction_Template> NewNodeAction;
 
             NewNodeAction = MakeShareable(new FEditorGraph_NewNode_SchemaAction_Template(BlueprintClassData.GetCategory(), FText::Format(MenuDesc, Arguments), FText::Format(ToolTip, Arguments), 0, BlueprintClassData.GetClass()));
 
             BlueprintBuilder.AddAction(NewNodeAction);
 		}
 	}
 
 	ContextMenuBuilder.Append(BlueprintBuilder);
 }
 
 void UEditorGraphSchema_Template::GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
 {
     Super::GetContextMenuActions(Menu, Context);
 }

 const FPinConnectionResponse UEditorGraphSchema_Template::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
 {
 	if(!(A && B))
 		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Both pins must be available."));
 
 	if (A->GetOwningNode() == B->GetOwningNode())
 		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect a node to itself."));
 
 	if (A->Direction == EGPD_Input && B->Direction == EGPD_Input)
 		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect an input pin to another input pin."));
 
 	if (A->Direction == EGPD_Output && B->Direction == EGPD_Output)
 		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("You can't connect an output pin to another output pin"));
 
 	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
 }
 
 FConnectionDrawingPolicy * UEditorGraphSchema_Template::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect & InClippingRect, FSlateWindowElementList & InDrawElements, UEdGraph * InGraphObj) const
 {
 	return new FEditorConnectionDrawingPolicy_Template(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
 }
 
 void UEditorGraphSchema_Template::CreateDefaultNodesForGraph(UEdGraph & Graph) const
 {
 	if (Graph.Nodes.Num() == 0)
 	{
 		UBP_Graph_Template* GraphAsset = Cast<UBP_Graph_Template>(Graph.GetOuter());
 
 		GraphAsset->Modify();
 		Graph.Modify();
 		
 		UBP_GraphNode_Template* AssetNode = GraphAsset->SpawnNodeInsideGraph<UBP_GraphNode_Template>(UBP_GraphNode_Template::StaticClass());
 
 		FGraphNodeCreator<UEditor_GraphNode_Template>Creator(Graph);
 		UEditor_GraphNode_Template* EdNode = Creator.CreateNode();
 		EdNode->SetAssetNode(AssetNode);
 		EdNode->AllocateDefaultPins();
 
 		Creator.Finalize();
 
 		EdNode->NodePosX = 0;
 		EdNode->NodePosY = 0;
 	}
 }
#undef LOCTEXT_NAMESPACE