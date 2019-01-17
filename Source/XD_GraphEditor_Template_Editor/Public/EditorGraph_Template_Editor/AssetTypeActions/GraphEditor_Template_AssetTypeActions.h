// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * 
 */
class FGraphEditor_Template_AssetTypeActions : public FAssetTypeActions_Base
{
public:
	FGraphEditor_Template_AssetTypeActions();

	// Inherited via FAssetTypeActions_Base
	virtual FText GetName() const override;
	virtual UClass * GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

};
