  // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TFGameMode.generated.h"

namespace MatchState {
	extern UNREALPROJECT_7A_API const FName CoolDown; //��� �ð��� ������ ���ڸ� �����ϴ� ����
}
/**
 * ���� ��� Ŭ����
 */
UCLASS()
class UNREALPROJECT_7A_API ATFGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ATFGameMode();
	// �÷��̾ ����(���)�Ǿ��� �� ȣ��Ǵ� �Լ�
	virtual void PlayerEliminated(class ATimeFractureCharacter* ElimmedCharacter, 
		class ATFPlayerController* VictimController, class ATFPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, class AController* ElimmedController);
	virtual void Tick(float DeltaTime) override;
	void WarmupToStartMatch();
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; } // ī��Ʈ�ٿ� �ð� ��ȯ �Լ�
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f; // ���� ���� �� ��� �ð�
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 60.f; // ���� ���� �� ��� �ð�
	UPROPERTY(EditDefaultsOnly)
	float CoolDownTime = 10.f; // ��� ���� �� ���ڸ� �����ϴ� �ð�
	float LevelStartingTime = 0.f; // ���� ���� �ð�
private:
	float CountdownTime = 5.f; // ī��Ʈ�ٿ� �ð�
	bool bFirstWarmupDone = false;
protected:
	virtual void BeginPlay() override; // ���� ���� �� ȣ��Ǵ� �Լ�
	virtual void OnMatchStateSet() override; // ��ġ ���°� ����� �� ȣ��Ǵ� �Լ�
};
