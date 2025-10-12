// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TFPlayerState.generated.h"

/**
 * ATFPlayerState
 * -------------------------------------------------------------
 * �� �÷��̾��� ����, óġ ��(Defeats), �̸� ���� ���� ������ �����ϰ�
 * ����-Ŭ���̾�Ʈ �� ������ �����Ѵ�.
 * HUD ���Ű� ���ھ�� ������Ʈ ��ɵ� ���Եȴ�.
 */
UCLASS()
class UNREALPROJECT_7A_API ATFPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/* ============================ */
	/*   �������̵� �� �⺻ ����   */
	/* ============================ */

	virtual void OnRep_Score() override;
	// ������ �������� ����Ǿ� Ŭ���̾�Ʈ�� ������ �� ȣ��ȴ�.
	// HUD�� ������ ��� �����ϰ� ���ھ�嵵 ���ΰ�ħ�Ѵ�.

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// ��Ʈ��ũ ������ ������ ����Ѵ�. (Defeats ��)

	/* ============================ */
	/*   ���� �� óġ ���� �Լ�    */
	/* ============================ */

	void AddToScore(float ScoreAmount);
	// ���� ������ �� ������ �����ϰ�, HUD ������ ���� OnRep_Score�� ���� ȣ���Ѵ�.

	UFUNCTION()
	virtual void OnRep_Defeats();
	// Defeats(óġ ��)�� ������ �� ȣ��Ǿ� HUD�� ���ھ�带 �����Ѵ�.

	void AddToDefeats(int32 DefeatAmount);
	// óġ ���� ������Ű�� HUD�� ��� �ݿ��Ѵ�.

	virtual void OnRep_PlayerName() override;
	// �÷��̾� �̸��� ������ �� ȣ��Ǿ� OverheadWidget�� �̸� �ݿ� �� ���ھ�� ������ �����Ѵ�.

	FORCEINLINE int32 GetDefeats() const { return Defeats; }
	// ���� óġ �� ��ȯ

private:
	/* ============================ */
	/*   ���� ���� �� ������ ����   */
	/* ============================ */

	UPROPERTY()
	class ATimeFractureCharacter* TFCharacter;
	// �� PlayerState�� ����� ĳ���� �ν��Ͻ� ������

	UPROPERTY()
	class ATFPlayerController* TFPlayerController;
	// ����� �÷��̾� ��Ʈ�ѷ� ������

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
	// óġ �� (��Ʈ��ũ ���� ���)
	UFUNCTION(Client, Reliable)
	void Client_PushKillsToPlayFab(int32 NewKills);
};
