// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"
#include "EditorGraph_Template/Nodes/BP_GraphNode_Template.h"

#define LOCTEXT_NAMESPACE "BP_Graph_Template"

UBP_Graph_Template::UBP_Graph_Template()
{
    
#if WITH_EDITORONLY_DATA
	EdGraph = nullptr;
#endif

}

void UBP_Graph_Template::InitGraph(UObject * ParentObject)
{
    Owner = ParentObject;
}

#if WITH_EDITORONLY_DATA
void UBP_Graph_Template::AddNode(UBP_GraphNode_Template * InNode)
{
	InNode->SetGraph(this);
	InNode->Rename(nullptr, this);
	int32 Index = Nodes.Add(InNode);
}

void UBP_Graph_Template::RemoveNode(UBP_GraphNode_Template * NodeToRemove)
{
	for (UBP_GraphNode_Template* Node : Nodes)
	{
		Node->RemoveLinkedNode(NodeToRemove);
	}

	int32 Removed=Nodes.Remove(NodeToRemove);
}
#endif

#undef LOCTEXT_NAMESPACE