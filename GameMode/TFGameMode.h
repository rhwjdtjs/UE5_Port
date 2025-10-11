// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TFGameMode.generated.h"

// 경기 상태 관리용 네임스페이스 (MatchState)
// - CoolDown: 경기 종료 후 승자 결정 및 결과 대기 상태
namespace MatchState {
	extern UNREALPROJECT_7A_API const FName CoolDown;
}

/**
 * ATFGameMode
 *
 * 게임 전반의 흐름을 관리하는 클래스.
 * - 웜업(Warmup), 본 경기(Match), 쿨다운(Cooldown) 등 상태 전환 처리
 * - 플레이어 제거, 리스폰, 점수 관리 기능 제공
 */
UCLASS()
class UNREALPROJECT_7A_API ATFGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ATFGameMode();

	// 경기 중 플레이어 제거 처리
	virtual void PlayerEliminated(class ATimeFractureCharacter* ElimmedCharacter,
		class ATFPlayerController* VictimController, class ATFPlayerController* AttackerController);

	// 플레이어 리스폰 요청 처리
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, class AController* ElimmedController);

	// 매 프레임 호출되는 게임 루프
	virtual void Tick(float DeltaTime) override;

	// 웜업 → 경기 시작 시점 처리
	void WarmupToStartMatch();

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }

protected:
	virtual void BeginPlay() override;          // 게임 시작 시 호출
	virtual void OnMatchStateSet() override;    // 매치 상태 변경 시 호출

public:
	// 경기 시간 관련 설정
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;     // 경기 시작 전 대기 시간

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 60.f;      // 실제 경기 시간

	UPROPERTY(EditDefaultsOnly)
	float CoolDownTime = 10.f;   // 경기 종료 후 대기 시간

	float LevelStartingTime = 0.f;  // 레벨이 시작된 시간
	float CountdownTime = 5.f;      // 현재 남은 카운트다운
	bool bFirstWarmupDone = false;  // 첫 웜업 여부 플래그
};
