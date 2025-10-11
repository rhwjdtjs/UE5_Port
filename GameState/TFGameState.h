// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TFGameState.generated.h"

/**
 * ATFGameState
 *
 * ������ ���� ���¸� �����ϴ� Ŭ����.
 * - �÷��̾� ���� �� �ְ� ������ ������ ����-Ŭ���̾�Ʈ �� ����ȭ�Ѵ�.
 * - ��Ƽ�÷��� ȯ�濡�� �ǽð� ��ŷ ���� ��� ���.
 */
UCLASS()
class UNREALPROJECT_7A_API ATFGameState : public AGameState
{
	GENERATED_BODY()
public:
	// ����(Replicate)�� ������ �����ϴ� �Լ� (UE ��Ʈ��ũ �ý���)
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// �ְ� ������ �����ϰ� TopScorePlayers ����� ������Ʈ�Ѵ�.
	void UpdateTopScorePlayers(class ATFPlayerState* ScoringPlayer);

	// ���� �ְ� ������ ������ �÷��̾� ��� (��� Ŭ���̾�Ʈ�� ������)
	UPROPERTY(Replicated)
	TArray<ATFPlayerState*> TopScorePlayers;

private:
	// ���� ���� �� �ְ� ����
	float TopScore = 0.f;
};
