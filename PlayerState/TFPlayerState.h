// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TFPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API ATFPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;
	void AddToScore(float ScoreAmount);
private:
	class ATimeFractureCharacter* TFCharacter; //캐릭터의 포인터를 저장한다.
	class ATFPlayerController* TFPlayerController; //플레이어 컨트롤러의 포인터를 저장한다.
};
