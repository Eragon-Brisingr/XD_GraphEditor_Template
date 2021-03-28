// Fill out your copyright notice in the Description page of Project Settings.
#include "EditorGraph_Template/Nodes/BP_GraphNode_Template.h"
#include "EditorGraph_Template/Graphs/BP_Graph_Template.h"

#define LOCTEXT_NAMESPACE "BP_GraphNode_Template" 

UBP_GraphNode_Template::UBP_GraphNode_Template()
{
	
}

UBP_GraphNode_Template * UBP_GraphNode_Template::GetNodePointer_Implementation()
{
	return this;
}

void UBP_GraphNode_Template::SetGraph(UBP_Graph_Template * InGraph)
{
	Graph = InGraph;
}

UBP_Graph_Template * UBP_GraphNode_Template::GetGraph()
{
	return Graph;
}

FText UBP_GraphNode_Template::GetNodeTitle_Implementation()
{
#if WITH_EDITOR
	return GetClass()->GetDisplayNameText();
#else
	return FText::GetEmpty();
#endif
}

TArray<UBP_GraphNode_Template*> UBP_GraphNode_Template::GetChildren()
{
	TArray<UBP_GraphNode_Template*>ReturnArray;
	for (UBP_GraphNode_Template* Node : ChildNodes)
	{
        auto* object = Node->GetNodePointer();
		if(object)ReturnArray.Add(object);
	}
	return ReturnArray;
}

TArray<UBP_GraphNode_Template*> UBP_GraphNode_Template::GetParents()
{
	return ParentNodes;
}

bool UBP_GraphNode_Template::HasInputPins_Implementation()
{
	return bHasInputPins;
}

bool UBP_GraphNode_Template::HasOutputPins_Implementation()
{
	return bHasOutputPins;
}

void UBP_GraphNode_Template::AddToChildren(UBP_GraphNode_Template * NewSubNode)
{
	ChildNodes.Add(NewSubNode);
}

void UBP_GraphNode_Template::AddToParent(UBP_GraphNode_Template * NewParentNode)
{
	ParentNodes.Add(NewParentNode);
}

bool UBP_GraphNode_Template::HasParentNodes()
{
	return ParentNodes.Num() > 0;
}

void UBP_GraphNode_Template::LinkArgumentNodeAsChild(UBP_GraphNode_Template * Child)
{
	AddToChildren(Child);
	Child->AddToParent(this);
}

void UBP_GraphNode_Template::ClearLinks()
{
	ParentNodes.Empty();
	ChildNodes.Empty();
}

bool UBP_GraphNode_Template::RemoveLinkedNode(UBP_GraphNode_Template * NodeToRemove)
{
	return RemoveNodeFromParents(NodeToRemove) || RemoveNodeFromChilds(NodeToRemove);
}

bool UBP_GraphNode_Template::RemoveNodeFromParents(UBP_GraphNode_Template * NodeToRemove)
{
	return ParentNodes.Remove(NodeToRemove) >= 0;
}

bool UBP_GraphNode_Template::RemoveNodeFromChilds(UBP_GraphNode_Template * NodeToRemove)
{
	return ChildNodes.Remove(NodeToRemove) >= 0;
}

#undef LOCTEXT_NAMESPACE