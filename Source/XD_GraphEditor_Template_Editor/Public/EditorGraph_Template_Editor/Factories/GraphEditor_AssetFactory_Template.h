// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "GraphEditor_AssetFactory_Template.generated.h"

/**
 * 
 */
UCLASS()
class UGraphEditor_AssetFactory_Template : public UFactory
{
	GENERATED_BODY()
	
public:
	UGraphEditor_AssetFactory_Template();
	/**
	* Create a new object by class.
	*
	* @param InClass
	* @param InParent
	* @param InName
	* @param Flags
	* @param Context
	* @param Warn
	* @return The new object.
	*/
	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	
	
};
