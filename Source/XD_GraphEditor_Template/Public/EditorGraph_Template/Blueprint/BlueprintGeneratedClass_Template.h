// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "BlueprintGeneratedClass_Template.generated.h"

/**
 * 
 */
USTRUCT()
struct XD_GRAPHEDITOR_TEMPLATE_API FDelegateRuntimeBinding_Template
{
	GENERATED_BODY()
public:
	/** The widget that will be bound to the live data. */
	UPROPERTY()
	FString ObjectName;

	/** The function or property we're binding to on the source object. */
	UPROPERTY()
	FName FunctionName;
};


UCLASS()
class XD_GRAPHEDITOR_TEMPLATE_API UBlueprintGeneratedClass_Template : public UBlueprintGeneratedClass
{
	GENERATED_BODY()
public:
	UPROPERTY()
	class UBP_Graph_Template* DesignerGraph_Template;

	UPROPERTY()
	TArray<FDelegateRuntimeBinding_Template> Bindings;
};
