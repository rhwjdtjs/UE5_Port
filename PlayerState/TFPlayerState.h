// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TFPlayerState.generated.h"

/**
 * ATFPlayerState
 * -------------------------------------------------------------
 * 각 플레이어의 점수, 처치 수(Defeats), 이름 등의 상태 정보를 저장하고
 * 서버-클라이언트 간 복제를 관리한다.
 * HUD 갱신과 스코어보드 업데이트 기능도 포함된다.
 */
UCLASS()
class UNREALPROJECT_7A_API ATFPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/* ============================ */
	/*   오버라이드 및 기본 동작   */
	/* ============================ */

	virtual void OnRep_Score() override;
	// 점수가 서버에서 변경되어 클라이언트에 복제될 때 호출된다.
	// HUD의 점수를 즉시 갱신하고 스코어보드도 새로고침한다.

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// 네트워크 복제할 변수를 등록한다. (Defeats 등)

	/* ============================ */
	/*   점수 및 처치 관리 함수    */
	/* ============================ */

	void AddToScore(float ScoreAmount);
	// 서버 권한일 때 점수를 누적하고, HUD 갱신을 위해 OnRep_Score를 직접 호출한다.

	UFUNCTION()
	virtual void OnRep_Defeats();
	// Defeats(처치 수)가 복제될 때 호출되어 HUD와 스코어보드를 갱신한다.

	void AddToDefeats(int32 DefeatAmount);
	// 처치 수를 증가시키고 HUD에 즉시 반영한다.

	virtual void OnRep_PlayerName() override;
	// 플레이어 이름이 복제될 때 호출되어 OverheadWidget에 이름 반영 및 스코어보드 갱신을 수행한다.

	FORCEINLINE int32 GetDefeats() const { return Defeats; }
	// 현재 처치 수 반환

private:
	/* ============================ */
	/*   내부 참조 및 데이터 관리   */
	/* ============================ */

	UPROPERTY()
	class ATimeFractureCharacter* TFCharacter;
	// 이 PlayerState가 연결된 캐릭터 인스턴스 포인터

	UPROPERTY()
	class ATFPlayerController* TFPlayerController;
	// 연결된 플레이어 컨트롤러 포인터

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
	// 처치 수 (네트워크 복제 대상)
	UFUNCTION(Client, Reliable)
	void Client_PushKillsToPlayFab(int32 NewKills);
};
