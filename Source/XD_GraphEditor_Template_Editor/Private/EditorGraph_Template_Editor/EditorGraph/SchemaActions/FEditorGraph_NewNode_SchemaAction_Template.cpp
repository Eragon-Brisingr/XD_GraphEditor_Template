// Copyright (c) 2018 Jhonny Hueller
#include "FEditorGraph_NewNode_SchemaAction_Template.h"
#include "BP_Graph_Template.h"
#include "Editor_GraphNode_Template.h"
#include "EdGraph/EdGraph.h"

#define LOCTEXT_NAMESPACE "EditorGraph_TemplateSchemaAction_NewNode"

UEdGraphNode * FEditorGraph_NewNode_SchemaAction_Template::PerformAction(UEdGraph * ParentGraph, UEdGraphPin * FromPin, const FVector2D Location, bool bSelectNewNode)
{
	check(ParentGraph);

	ParentGraph->Modify();


	UBP_Graph_Template* Graph_Template = CastChecked<UBP_Graph_Template>(ParentGraph->GetOuter());
	Graph_Template->Modify();

	UBP_GraphNode_Template* AssetNode = Graph_Template->SpawnNodeInsideGraph<UBP_GraphNode_Template>(NewNodeClass);

	UEdGraphNode* EditorNode = CreateEditorNode(ParentGraph, bSelectNewNode, AssetNode);

	//EditorNode->AllocateDefaultPins();   for some reason it was called 2 times even if I only call it here
	EditorNode->AutowireNewNode(FromPin);
	EditorNode->NodePosX = Location.X;
	EditorNode->NodePosY = Location.Y;

	return EditorNode;
}

UEdGraphNode * FEditorGraph_NewNode_SchemaAction_Template::CreateEditorNode(UEdGraph * ParentGraph, bool bSelectNewNode, UBP_GraphNode_Template* AssetNode)
{
    FGraphNodeCreator<UEditor_GraphNode_Template>Creator(*ParentGraph);
    UEditor_GraphNode_Template* EdNode = Creator.CreateNode(bSelectNewNode);
    EdNode->SetAssetNode(AssetNode);
    Creator.Finalize();
    return EdNode;
}
#undef LOCTEXT_NAMESPACE