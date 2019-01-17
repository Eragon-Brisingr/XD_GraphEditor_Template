// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BP_GraphNode_Template.generated.h"

#define LOCTEXT_NAMESPACE "BP_GraphNode_Template"

class UBP_Graph_Template;
/**
 * Base abstract class for every node of the graph.
 */
UCLASS(Blueprintable, BlueprintType)
class XD_GRAPHEDITOR_TEMPLATE_API UBP_GraphNode_Template : public UObject
{
	GENERATED_BODY()
	
public:
	UBP_GraphNode_Template();

	UFUNCTION(BlueprintPure, Category = "Node Data")
	TArray<UBP_GraphNode_Template*> GetChildren();

	UFUNCTION(BlueprintPure, Category = "Node Data")
	TArray<UBP_GraphNode_Template*> GetParents();

    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Node Data")
	FText GetNodeTitle();
    virtual FText GetNodeTitle_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Node Data")
	bool HasInputPins();

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Node Data")
	bool HasOutputPins();

	UFUNCTION(BlueprintNativeEvent, Category = "Node Data")
	UBP_GraphNode_Template* GetNodePointer();
    virtual UBP_GraphNode_Template* GetNodePointer_Implementation();

	virtual void SetGraph(UBP_Graph_Template* InGraph);
	virtual UBP_Graph_Template* GetGraph();
	virtual void AddToChildren(UBP_GraphNode_Template* NewChildNode);
	virtual void AddToParent(UBP_GraphNode_Template* NewParentNode);
	virtual bool HasParentNodes();
	virtual void LinkArgumentNodeAsChild(UBP_GraphNode_Template* Child);
	virtual void ClearLinks();
	virtual bool RemoveLinkedNode(UBP_GraphNode_Template* NodeToRemove);
	virtual bool RemoveNodeFromParents(UBP_GraphNode_Template* NodeToRemove);
	virtual bool RemoveNodeFromChilds(UBP_GraphNode_Template* NodeToRemove);

	UPROPERTY()
	uint8 bIsVariable : 1;
protected:
	UPROPERTY()
	UBP_Graph_Template * Graph = nullptr;
	UPROPERTY()
	TArray<UBP_GraphNode_Template*> ParentNodes;
	UPROPERTY()
	TArray<UBP_GraphNode_Template*> ChildNodes;
    UPROPERTY()
	bool bHasInputPins = true;
    UPROPERTY()
	bool bHasOutputPins = true;

public:
	UPROPERTY(EditAnywhere, Category = "Example")
	uint8 bExampleVar : 1;

	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FExampleBindDelegate);
	UPROPERTY()
	FExampleBindDelegate bExampleVarDelegate;
};

#undef LOCTEXT_NAMESPACE