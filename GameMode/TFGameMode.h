// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TFGameMode.generated.h"

/**
 * 게임 모드 클래스
 */
UCLASS()
class UNREALPROJECT_7A_API ATFGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	// 플레이어가 제거(사망)되었을 때 호출되는 함수
	virtual void PlayerEliminated(class ATimeFractureCharacter* ElimmedCharacter, 
		class ATFPlayerController* VictimController, class ATFPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, class AController* ElimmedController);
};
