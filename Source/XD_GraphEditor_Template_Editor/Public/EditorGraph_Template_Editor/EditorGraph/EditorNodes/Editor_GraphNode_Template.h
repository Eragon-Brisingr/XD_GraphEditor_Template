// Copyright (c) 2018 Jhonny Hueller


#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraphNode.h"
#include "BP_GraphNode_Template.h"
#include "SubclassOf.h"
#include "Editor_GraphNode_Template.generated.h"

/**
 * 
 */
UCLASS()
class UEditor_GraphNode_Template : public UEdGraphNode
{
	GENERATED_UCLASS_BODY()
	
public:
	// Inherited via EdGraphNode.h
	TSharedPtr<SGraphNode> CreateVisualWidget() override; 	/** Create a visual widget to represent this node in a graph editor or graph panel.  If not implemented, the default node factory will be used. */
	void AllocateDefaultPins() override;
	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	void PrepareForCopying() override;
	void DestroyNode() override;
	void AutowireNewNode(UEdGraphPin* FromPin) override;
	void GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const override;

public:
	virtual void SetAssetNode(UBP_GraphNode_Template* InNode);
	virtual UBP_GraphNode_Template* GetAssetNode();

	virtual void PostCopyNode();

	virtual bool RenameUniqueNode(const FText& NewName);

	virtual FText GetEdNodeName() const;
	virtual void SetEdNodeName(const FText& Name);
	virtual void SetEdNodeName(const FName& Name);

	virtual TSharedPtr<SWidget> GetContentWidget();

	virtual void UpdateVisualNode();

    virtual void SaveNodesAsChildren(TArray<UEdGraphNode*>& Children);
protected:
	virtual bool HasOutputPins();
	virtual bool HasInputPins();
	TSharedPtr<SGraphNode> SlateNode;
public:
	UPROPERTY(Instanced)
	UBP_GraphNode_Template* BP_Node_Template = nullptr;

protected:
	UPROPERTY()
	FText EdNodeName;

};
