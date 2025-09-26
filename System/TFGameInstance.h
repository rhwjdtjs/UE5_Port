// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TFGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API UTFGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UPROPERTY(EditDefaultsOnly, Category="LoadingScreen")
	float MinimumDisplayTime = 0.f;
	UPROPERTY(EditDefaultsOnly, Category = "LoadingScreen")
	FString MovieName = TEXT("LoadingVideo");

private:
	void OnPreLoadMap(const FString& MapName);
	void OnPostLoadMap(class UWorld* LoadedWorld);
};
