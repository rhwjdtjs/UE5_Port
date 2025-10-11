// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TFGameState.generated.h"

/**
 * ATFGameState
 *
 * 게임의 전역 상태를 관리하는 클래스.
 * - 플레이어 점수 및 최고 점수자 정보를 서버-클라이언트 간 동기화한다.
 * - 멀티플레이 환경에서 실시간 랭킹 관리 기능 담당.
 */
UCLASS()
class UNREALPROJECT_7A_API ATFGameState : public AGameState
{
	GENERATED_BODY()
public:
	// 복제(Replicate)할 변수를 정의하는 함수 (UE 네트워크 시스템)
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 최고 점수를 갱신하고 TopScorePlayers 목록을 업데이트한다.
	void UpdateTopScorePlayers(class ATFPlayerState* ScoringPlayer);

	// 현재 최고 점수를 보유한 플레이어 목록 (모두 클라이언트에 복제됨)
	UPROPERTY(Replicated)
	TArray<ATFPlayerState*> TopScorePlayers;

private:
	// 현재 게임 내 최고 점수
	float TopScore = 0.f;
};
