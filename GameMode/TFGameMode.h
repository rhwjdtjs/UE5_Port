  // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TFGameMode.generated.h"

namespace MatchState {
	extern UNREALPROJECT_7A_API const FName CoolDown; //경기 시간이 끝나고 승자를 결정하는 상태
}
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
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; } // 카운트다운 시간 반환 함수
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f; // 게임 시작 전 대기 시간
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 60.f; // 게임 시작 전 대기 시간
	UPROPERTY(EditDefaultsOnly)
	float CoolDownTime = 10.f; // 경기 종료 후 승자를 결정하는 시간
	float LevelStartingTime = 0.f; // 레벨 시작 시간
private:
	float CountdownTime = 5.f; // 카운트다운 시간
	bool bFirstWarmupDone = false;
protected:
	virtual void BeginPlay() override; // 게임 시작 시 호출되는 함수
	virtual void OnMatchStateSet() override; // 매치 상태가 변경될 때 호출되는 함수
};
