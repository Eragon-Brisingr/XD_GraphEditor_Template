// Copyright (c) 2018 Jhonny Hueller


#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "EditorGraph_Template.generated.h"

/**
 * 
 */
class UBP_Graph_Template;

UCLASS()
class UEditorGraph_Template : public UEdGraph
{
	GENERATED_BODY()
	
public:

	virtual bool IsNameUnique(const FText& InName);
	virtual void SaveGraph();
	virtual void ClearOldLinks();
	virtual void LinkAssetNodes();
	virtual void RefreshNodes();
	virtual void MapNamedNodes();
	virtual UBP_Graph_Template* GetGraphAsset();
};
