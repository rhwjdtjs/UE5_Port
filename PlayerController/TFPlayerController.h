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
	//ui
	void UpdateScoreboard();
	UFUNCTION(Client, Reliable)
	void ClientAddKillFeedMessage(const FString& Killer, const FString& Victim);
	void AddKillFeedMessage(const FString& Killer, const FString& Victim);
	virtual void SetupInputComponent() override; // �Է� ������Ʈ�� �����ϴ� �Լ�
	void ShowReturnToMainMenu(); // ���� �޴��� ���ư��� ������ ǥ���ϴ� �Լ�
	UPROPERTY(EditAnywhere, Category = UI)
	TSubclassOf<class UUserWidget> ReturnToMainMenuClass; // ���� �޴��� ���ư��� ���� Ŭ����
	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu; // ���� �޴��� ���ư��� ���� �ν��Ͻ�
	bool bReturnToMainMenuOpen = false; // ���� �޴��� ���ư��� ������ �����ִ��� ����
	//���
	void SetHUDHealth(float Health, float MaxHealth); // ����� ü���� �����ϴ� �Լ�
	void SetHUDShield(float Shield, float MaxShield); // ����� ü���� �����ϴ� �Լ�
	void SetHUDScore(float Score); // ����� ������ �����ϴ� �Լ�
	void SetHUDDefeats(int32 Defeats); // ����� óġ ���� �����ϴ� �Լ�
	void SetHUDWeaponAmmo(int32 Ammos); // ����� óġ ���� �����ϴ� �Լ�
	void SetHUDCarriedAmmo(int32 Ammos); // ����� ���� ź���� �����ϴ� �Լ�
	void SetHUDMatchCountdown(float CountdownTime); // ����� ��ġ ī��Ʈ�ٿ��� �����ϴ� �Լ� 
	void SetHUDAlertCountDown(float CountdownTime); // ����� ī��Ʈ�ٿ��� �����ϴ� �Լ�
	void SetHUDGrenadeCount(int32 Grenades); // ����� ����ź ������ �����ϴ� �Լ�
	virtual void OnPossess(APawn* InPawn) override; // �÷��̾ Pawn�� ������ �� ȣ��Ǵ� �Լ�
	virtual void Tick(float DeltaTime) override; // �� �����Ӹ��� ȣ��Ǵ� �Լ�
	virtual void ReceivedPlayer() override; // �÷��̾ ��Ʈ�ѷ��� �޾��� �� ȣ��Ǵ� �Լ�
	virtual float GetServerTime(); // ���� �ð��� �������� �Լ�
	void OnMatchStateSet(FName State); // ��ġ ���°� ����� �� ȣ��Ǵ� �Լ�
	void HandleCoolDown(); // ��ٿ� ���¸� ó���ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& Message); //0913 ä��

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& Sender, const FString& Message);//0913 ä��
	UFUNCTION(Client, Reliable)
	void ClientPlayHitConfirmSound(class USoundCue* HitSound); //0914 hitsound
private:
	UPROPERTY()
	class ATFHUD* TfHud; // ����ĳ���� ���	
	UPROPERTY()
	class ATFGameMode* TFGameMode; // ���� ��� Ŭ���� ������
	float LevelStartingTime = 0.f; // ���� ���� �ð�
	uint32 CountdownInt; // ī��Ʈ�ٿ� �ð� (�� ����)
	float MatchTime = 0.f; // ��ġ �ð� (�� ����)
	float WarmupTime = 0.f; // ���� �ð� (�� ����)
	float CoolDownTime = 0.f; // ��ٿ� �ð� (�� ����)
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)

	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState(); // ��ġ ���°� ����Ǿ��� �� ȣ��Ǵ� �Լ�
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay; // ĳ���� �������� ����
	bool bInitializeHealth = false; // ����� ü���� �ʱ�ȭ�Ǿ����� ����
	bool bInitializeShield = false; // ����� ���尡 �ʱ�ȭ�Ǿ����� ����
	bool bInitializeScore = false; // ����� ������ �ʱ�ȭ�Ǿ����� ����
	bool bInitializeDefeats = false; // ����� óġ ���� �ʱ�ȭ�Ǿ����� ����
	bool bInitializeGrenades = false; // ����� ����ź ������ �ʱ�ȭ�Ǿ����� ����
	bool bInitializeAmmos = false; // ����� ź���� �ʱ�ȭ�Ǿ����� ����
	bool bInitializeCarriedAmmos = false; // ����� ���� ź���� �ʱ�ȭ�Ǿ����� ����
	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	float HUDDefeats;
	float HUDAmmos;
	float HUDCarriedAmmos;
	int32 HUDGrenades;
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ScoreboardClass;      // WBP_Scoreboard

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ScoreboardRowClass;   // WBP_ScoreboardRow
	UPROPERTY()
	UUserWidget* ScoreboardWidget;

	UFUNCTION()
	void ShowScoreboard();

	UFUNCTION()
	void HideScoreboard();

protected:
	virtual void BeginPlay() override; // �÷��̾� ��Ʈ�ѷ��� ���۵� �� ȣ��Ǵ� �Լ�
	void SetHUDTime(); // HUD�� �ð��� �����ϴ� �Լ�
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//�����ϴ� �׸��� �����ϴ� �Լ�
	void PollInit(); // ���� ���� �Լ� �ʱ�ȭ
	void HandleMatchHasStarted(); // ��ġ�� ���۵Ǿ��� �� ȣ��Ǵ� �Լ�
	//
	//Ŭ���̾�Ʈ�� ���� �ð� ���߱�
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState(); // ������ ��ġ ���¸� Ȯ��
	UFUNCTION(Client, Reliable)
	void ClientJoinMatch(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown); // Ŭ���̾�Ʈ�� ��ġ ���¸� Ȯ��
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
