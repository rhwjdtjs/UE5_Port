// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TFGameMode.generated.h"

/**
 * ���� ��� Ŭ����
 */
UCLASS()
class UNREALPROJECT_7A_API ATFGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	// �÷��̾ ����(���)�Ǿ��� �� ȣ��Ǵ� �Լ�
	virtual void PlayerEliminated(class ATimeFractureCharacter* ElimmedCharacter, 
		class ATFPlayerController* VictimController, class ATFPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, class AController* ElimmedController);
};
