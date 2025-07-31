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
	ATFGameMode();
	// 플레이어가 제거(사망)되었을 때 호출되는 함수
	virtual void PlayerEliminated(class ATimeFractureCharacter* ElimmedCharacter, 
		class ATFPlayerController* VictimController, class ATFPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, class AController* ElimmedController);
	virtual void Tick(float DeltaTime) override;
	void WarmupToStartMatch();
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f; // 게임 시작 전 대기 시간
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 60.f; // 게임 시작 전 대기 시간
	float LevelStartingTime = 0.f; // 레벨 시작 시간
private:
	float CountdownTime = 0.f; // 카운트다운 시간
protected:
	virtual void BeginPlay() override; // 게임 시작 시 호출되는 함수
	virtual void OnMatchStateSet() override; // 매치 상태가 변경될 때 호출되는 함수
	
};
