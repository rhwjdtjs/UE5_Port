// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LBGameMode.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API ALBGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PostLogin(APlayerController* NewPlayer) override; //플레이어 컨트롤러를 받아들인다.
};
