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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; //�����ϴ� �׸��� �����ϴ� �Լ�
	void UpdateTopScorePlayers(class ATFPlayerState* ScoringPlayer); //�ְ� ������ ���� �÷��̾���� ������Ʈ�ϴ� �Լ�
	UPROPERTY(Replicated)
	TArray<ATFPlayerState*> TopScorePlayers; //��	�� ���� ���¸� �����ϴ� �迭

	
private:
	float TopScore = 0.f; //�ְ� ����
};
