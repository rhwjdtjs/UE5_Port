// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TFGameState.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API ATFGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; //복제하는 항목을 정의하는 함수
	void UpdateTopScorePlayers(class ATFPlayerState* ScoringPlayer); //최고 점수를 가진 플레이어들을 업데이트하는 함수
	UPROPERTY(Replicated)
	TArray<ATFPlayerState*> TopScorePlayers; //어	떤 게임 상태를 저장하는 배열

	
private:
	float TopScore = 0.f; //최고 점수
};
