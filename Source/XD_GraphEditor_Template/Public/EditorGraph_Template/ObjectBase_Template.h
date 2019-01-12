// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ObjectBase_Template.generated.h"

class UBP_Graph_Template;

/**
 * 
 */
UCLASS(Abstract)
class XD_GRAPHEDITOR_TEMPLATE_API UObjectBase_Template : public UObject
{
	GENERATED_BODY()
public:
	UBP_Graph_Template* GetGraph_Template() const;
};
