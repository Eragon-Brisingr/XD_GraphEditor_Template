// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Templates/SubclassOf.h"
#include "BP_Graph_Template.generated.h"

#define LOCTEXT_NAMESPACE "BP_Graph_Template"

class UBP_GraphNode_Template;
class UStartNode;

/**
 * 
 */
UCLASS(Blueprintable,BlueprintType)
class XD_GRAPHEDITOR_TEMPLATE_API UBP_Graph_Template : public UObject
{
	GENERATED_BODY()
	
public:
	UBP_Graph_Template();

	UPROPERTY(BlueprintReadOnly, Category = "Graph Data")
	TArray<UBP_GraphNode_Template*> Nodes;

	UPROPERTY(BlueprintReadOnly, Category = "Graph Data")
	TMap<FString, UBP_GraphNode_Template*> NamedNodes;

    UPROPERTY(BlueprintReadOnly, Category = "Graph Data")
    TMap<UBP_GraphNode_Template*, FString> NodesNames;

    UPROPERTY(BlueprintReadWrite, Category = "Graph Data")
    UObject* Owner;

    UFUNCTION(BlueprintCallable, Category = "Graph Data")
    void InitGraph(UObject* ParentObject);

	TArray<UBP_GraphNode_Template*> GetAllNodes() const { return Nodes; }

#if WITH_EDITORONLY_DATA

public:

	UPROPERTY()
	class UEdGraph* EdGraph;

	template <class T>
	T* SpawnNodeInsideGraph(TSubclassOf<UBP_GraphNode_Template> InNodeClass)
	{
		T* SpawenNode = NewObject<T>(this, InNodeClass, NAME_None, RF_Transactional);
		AddNode(SpawenNode);
		return SpawenNode;
	}


	virtual void AddNode(UBP_GraphNode_Template* InNode);
	virtual void RemoveNode(UBP_GraphNode_Template* NodeToRemove);

#endif
};

#undef LOCTEXT_NAMESPACE