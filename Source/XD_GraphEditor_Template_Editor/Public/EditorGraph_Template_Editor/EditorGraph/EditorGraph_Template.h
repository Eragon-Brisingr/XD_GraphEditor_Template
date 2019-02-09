// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "EditorGraph_Template.generated.h"

/**
 * 
 */
class UBP_Graph_Template;
class FGraphEditor_Template;

UCLASS()
class UEditorGraph_Template : public UEdGraph
{
	GENERATED_BODY()
	
public:
	virtual void BuildGraph();
	virtual void ClearOldLinks();
	virtual void LinkAssetNodes();
	virtual void RefreshNodes();
	virtual void MapNamedNodes();
	virtual UBP_Graph_Template* GetGraphAsset();

	FGraphEditor_Template* OwningGraphEditor;
};
