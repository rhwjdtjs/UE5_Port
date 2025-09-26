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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movies")
	FString LogoMovie = TEXT("LogoMovie");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movies")
	FString LoadingVideo = TEXT("LoadingVideo");
	bool bFirstWarmupDone = false;
	// 로고 무비를 이미 재생했는지 체크
	bool bLogoPlayed = false;

private:
	void OnPreLoadMap(const FString& MapName);
	void OnPostLoadMap(class UWorld* LoadedWorld);
};
