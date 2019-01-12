// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KismetCompiler.h"

class UEditorGraph_Blueprint_Template;
class FCompilerResultsLog;
struct FKismetCompilerOptions;

/**
 * 
 */
class FBP_Compiler_Template : public FKismetCompilerContext
{
public:
	FBP_Compiler_Template(UEditorGraph_Blueprint_Template* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions, TArray<UObject*>* InObjLoaded);
	~FBP_Compiler_Template() override;

	// FKismetCompilerContext
	void SpawnNewClass(const FString& NewClassName) override;
	void FinishCompilingClass(UClass* Class) override;
	// End FKismetCompilerContext
};
