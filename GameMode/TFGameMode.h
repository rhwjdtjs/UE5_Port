// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TFGameMode.generated.h"

// ��� ���� ������ ���ӽ����̽� (MatchState)
// - CoolDown: ��� ���� �� ���� ���� �� ��� ��� ����
namespace MatchState {
	extern UNREALPROJECT_7A_API const FName CoolDown;
}

/**
 * ATFGameMode
 *
 * ���� ������ �帧�� �����ϴ� Ŭ����.
 * - ����(Warmup), �� ���(Match), ��ٿ�(Cooldown) �� ���� ��ȯ ó��
 * - �÷��̾� ����, ������, ���� ���� ��� ����
 */
UCLASS()
class UNREALPROJECT_7A_API ATFGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ATFGameMode();

	// ��� �� �÷��̾� ���� ó��
	virtual void PlayerEliminated(class ATimeFractureCharacter* ElimmedCharacter,
		class ATFPlayerController* VictimController, class ATFPlayerController* AttackerController);

	// �÷��̾� ������ ��û ó��
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, class AController* ElimmedController);

	// �� ������ ȣ��Ǵ� ���� ����
	virtual void Tick(float DeltaTime) override;

	// ���� �� ��� ���� ���� ó��
	void WarmupToStartMatch();

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }

protected:
	virtual void BeginPlay() override;          // ���� ���� �� ȣ��
	virtual void OnMatchStateSet() override;    // ��ġ ���� ���� �� ȣ��

public:
	// ��� �ð� ���� ����
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;     // ��� ���� �� ��� �ð�

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 60.f;      // ���� ��� �ð�

	UPROPERTY(EditDefaultsOnly)
	float CoolDownTime = 10.f;   // ��� ���� �� ��� �ð�

	float LevelStartingTime = 0.f;  // ������ ���۵� �ð�
	float CountdownTime = 5.f;      // ���� ���� ī��Ʈ�ٿ�
	bool bFirstWarmupDone = false;  // ù ���� ���� �÷���
};
