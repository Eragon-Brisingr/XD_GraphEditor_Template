// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"
#include "FEditorGraph_NewNode_SchemaAction_Template.generated.h"

class UBP_GraphNode_Template;

/**
 * 
 */
USTRUCT()
struct FEditorGraph_NewNode_SchemaAction_Template : public FEdGraphSchemaAction
{
public:
	GENERATED_USTRUCT_BODY()

	FEditorGraph_NewNode_SchemaAction_Template() :FEdGraphSchemaAction(), NewNodeClass(nullptr) {}
	FEditorGraph_NewNode_SchemaAction_Template(FText InNodeCategory, FText InMenuDesc, FText InToolTip, const int32 InGrouping, TSubclassOf<UBP_GraphNode_Template> InNodeClass) :FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping), NewNodeClass(InNodeClass) {}

	/** Execute this action, given the graph and schema, and possibly a pin that we were dragged from. Returns a node that was created by this action (if any). */
	UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;

    virtual UEdGraphNode* CreateEditorNode(UEdGraph * ParentGraph, bool bSelectNewNode, UBP_GraphNode_Template* AssetNode);
	TSubclassOf<UBP_GraphNode_Template> NewNodeClass;
};
