// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TFPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class UNREALPROJECT_7A_API ATFPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float Health, float MaxHealth); // ����� ü���� �����ϴ� �Լ�
	void SetHUDScore(float Score); // ����� ������ �����ϴ� �Լ�
	void SetHUDDefeats(int32 Defeats); // ����� óġ ���� �����ϴ� �Լ�
	void SetHUDWeaponAmmo(int32 Ammos); // ����� óġ ���� �����ϴ� �Լ�
	void SetHUDCarriedAmmo(int32 Ammos); // ����� ���� ź���� �����ϴ� �Լ�
	void SetHUDMatchCountdown(float CountdownTime); // ����� ��ġ ī��Ʈ�ٿ��� �����ϴ� �Լ� 
	virtual void OnPossess(APawn* InPawn) override; // �÷��̾ Pawn�� ������ �� ȣ��Ǵ� �Լ�
	virtual void Tick(float DeltaTime) override; // �� �����Ӹ��� ȣ��Ǵ� �Լ�
	virtual void ReceivedPlayer() override; // �÷��̾ ��Ʈ�ѷ��� �޾��� �� ȣ��Ǵ� �Լ�
	virtual float GetServerTime(); // ���� �ð��� �������� �Լ�
	void OnMatchStateSet(FName State); // ��ġ ���°� ����� �� ȣ��Ǵ� �Լ�
private:
	UPROPERTY()
	class ATFHUD* TfHud; // ����ĳ���� ���	
	uint32 CountdownInt; // ī��Ʈ�ٿ� �ð� (�� ����)
	float MatchTime = 60.f; // ��ġ �ð� (�� ����)
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)

	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState(); // ��ġ ���°� ����Ǿ��� �� ȣ��Ǵ� �Լ�
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay; // ĳ���� �������� ����
	bool bInitializeCharacterOverlay = false; // ĳ���� �������� �ʱ�ȭ ����
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	float HUDDefeats;
protected:
	virtual void BeginPlay() override; // �÷��̾� ��Ʈ�ѷ��� ���۵� �� ȣ��Ǵ� �Լ�
	void SetHUDTime(); // HUD�� �ð��� �����ϴ� �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//�����ϴ� �׸��� �����ϴ� �Լ�
	void PollInit(); // ���� ���� �Լ� �ʱ�ȭ
	//
	//Ŭ���̾�Ʈ�� ���� �ð� ���߱�
	UFUNCTION(Server, Reliable)
	void ServerRequestimeSync(float TimeOfClientRequest); // Ŭ���̾�Ʈ�� ������ ���� �ð��� ��û
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest); // ������ Ŭ���̾�Ʈ���� ���� �ð��� ����
	void CheckTimeSync(float DeltaTime); // �ð� ����ȭ Ȯ�� �Լ�
	UPROPERTY(EditAnywhere, Category=Time)
	float TimeSyncFrequency = 5.f; // �ð� ����ȭ �ֱ� (�� ����)
	float TimeSyncRunningTime = 0.f; // �ð� ����ȭ�� �󸶳� ����Ǿ����� �����ϴ� ����
	float ClientServerDelta = 0.f; // Ŭ���̾�Ʈ�� ���� ���� �ð� ���̸� �����ϴ� ����
	
};
