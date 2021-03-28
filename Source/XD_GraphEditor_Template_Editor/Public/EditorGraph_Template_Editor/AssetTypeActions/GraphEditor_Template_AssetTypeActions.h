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
	FText GetName() const override;
	UClass * GetSupportedClass() const override;
	FColor GetTypeColor() const override;
	uint32 GetCategories() override;
	bool HasActions(const TArray<UObject*>& InObjects) const override;
	void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
};
