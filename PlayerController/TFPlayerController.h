// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TFPlayerController.generated.h"

/**
 * �÷��̾��� �Է�, HUD ������Ʈ, ��ġ ����, ��Ʈ��ũ ����ȭ ���� �����ϴ� Ŭ����
 * Ŭ���̾�Ʈ-���� �� �ð� ����ȭ, UI ǥ��, ä��, ������ �� �ټ��� �ý����� ���� ����
 */

UCLASS()
class UNREALPROJECT_7A_API ATFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** ====================== ��Ʈ��ũ / ��ġ ���� ���� ====================== */

	// ���� ��ġ ���¸� ��Ÿ�� (WaitingToStart, InProgress, CoolDown ��)
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	// ��ġ ���°� ����Ǿ��� �� ȣ��Ǵ� �Լ�
	UFUNCTION()
	void OnRep_MatchState();

	// ��ġ ���°� �����ǰų� ����� �� UI �� ���� ó�� ����
	void OnMatchStateSet(FName State);

	// �������� �Ⱦ� ȿ��(����/����Ʈ)�� Ŭ���̾�Ʈ�� �����Ŵ
	UFUNCTION(Client, Reliable)
	void ClientPlayPickupEffects(USoundCue* Sound, class UNiagaraSystem* Effect, FVector Location, FRotator Rotation);

	// �������� �κ��� ���� ��ư�� Ȱ��ȭ�ϵ��� Ŭ���̾�Ʈ�� �˸�
	UFUNCTION(Client, Reliable)
	void ClientEnableStartButton();

	// Ŭ���̾�Ʈ�� ��ư�� ������ �� ������ ��ġ ���� ��û
	UFUNCTION(BlueprintCallable)
	void RequestStartMatch();


	/** ====================== UI / ���� ���� ====================== */

	// �κ� ���� Ŭ����
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ULobbyWidget> LobbyWidgetClass;

	// �κ� ���� �ν��Ͻ�
	UPROPERTY()
	class ULobbyWidget* LobbyWidget;

	// ������ UI ����
	void UpdateScoreboard();

	// ų�ǵ�(���� �α�)�� Ŭ���̾�Ʈ HUD�� �߰�
	UFUNCTION(Client, Reliable)
	void ClientAddKillFeedMessage(const FString& Killer, const FString& Victim);

	// ų�ǵ� �޽����� HUD�� ���� �߰�
	void AddKillFeedMessage(const FString& Killer, const FString& Victim);

	// �Է� ���ε� ����
	virtual void SetupInputComponent() override;

	// ���� �޴� ������ ���� ����
	void ShowReturnToMainMenu();

	// ���� �޴��� ���ư��� ���� Ŭ����
	UPROPERTY(EditAnywhere, Category = UI)
	TSubclassOf<class UUserWidget> ReturnToMainMenuClass;

	// ���� �޴��� ���ư��� ���� �ν��Ͻ�
	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	// �÷��̾ ų���� �� ǥ�õǴ� ����
	UFUNCTION(Client, Reliable)
	void ClientShowKillWidget();

	// �÷��̾ �׾��� �� ǥ�õǴ� ����
	UFUNCTION(Client, Reliable)
	void ClientShowKilledWidget();

	// ų ���� Ŭ����
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> KillWidgetClass;

	// ��� ���� Ŭ����
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> KilledWidgetClass;

	// ���� �޴��� ���� �ִ��� ����
	bool bReturnToMainMenuOpen = false;


	/** ====================== HUD ���� ====================== */

	// ü�� HUD ������Ʈ
	void SetHUDHealth(float Health, float MaxHealth);

	// ���� HUD ������Ʈ
	void SetHUDShield(float Shield, float MaxShield);

	// ���� HUD ������Ʈ
	void SetHUDScore(float Score);

	// ���� HUD ������Ʈ
	void SetHUDDefeats(int32 Defeats);

	// ���� ���� ź�� HUD ������Ʈ
	void SetHUDWeaponAmmo(int32 Ammos);

	// ���� ź�� HUD ������Ʈ
	void SetHUDCarriedAmmo(int32 Ammos);

	// ��ġ ī��Ʈ�ٿ� HUD ������Ʈ
	void SetHUDMatchCountdown(float CountdownTime);

	// �˸� ī��Ʈ�ٿ� HUD ������Ʈ
	void SetHUDAlertCountDown(float CountdownTime);

	// ����ź ���� HUD ������Ʈ
	void SetHUDGrenadeCount(int32 Grenades);


	/** ====================== �÷��̾� ���� / ����ȭ ====================== */

	// �÷��̾ Pawn�� ������ �� ȣ��
	virtual void OnPossess(APawn* InPawn) override;

	// �� �����Ӹ��� ȣ��
	virtual void Tick(float DeltaTime) override;

	// Ŭ���̾�Ʈ�� �÷��̾ �޾��� �� (��Ʈ��ũ �ʱ�ȭ ��)
	virtual void ReceivedPlayer() override;

	// ���� �ð� �������� (Ŭ��� ����ȭ�� ����)
	virtual float GetServerTime();

	// ��ٿ� ���� ó��
	void HandleCoolDown();

	// ä��: ������ �޽��� ����
	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& Message);

	// ä��: �����κ��� �޽��� ����
	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& Sender, const FString& Message);

	// ��Ʈ ���� ���
	UFUNCTION(Client, Reliable)
	void ClientPlayHitConfirmSound(class USoundCue* HitSound);

	// Ŭ���̾�Ʈ�� ��ġ�� �������� �� ���� ���� ����
	UFUNCTION(Client, Reliable)
	void ClientJoinMatch(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown);


private:
	/** ====================== ���� ������ / Ÿ�̸� ====================== */

	FTimerHandle KillFeedClearTimer;
	FTimerHandle PollInitTimerHandle;

	UPROPERTY()
	class UUserWidget* KillWidgetInstance;

	UPROPERTY()
	class UUserWidget* KilledWidgetInstance;

	UPROPERTY()
	class ATFHUD* TfHud;

	UPROPERTY()
	class ATFGameMode* TFGameMode;

	float LevelStartingTime = 0.f;
	uint32 CountdownInt;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CoolDownTime = 0.f;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	// HUD �ʱ�ȭ �÷��׵�
	bool bInitializeHealth = false;
	bool bInitializeShield = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenades = false;
	bool bInitializeAmmos = false;
	bool bInitializeCarriedAmmos = false;

	// HUD ����ȭ�� �ӽð���
	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	float HUDDefeats;
	float HUDAmmos;
	float HUDCarriedAmmos;
	int32 HUDGrenades;

	// ������ ����
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ScoreboardClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ScoreboardRowClass;

	UPROPERTY()
	UUserWidget* ScoreboardWidget;

	// ������ ����/�ݱ�
	UFUNCTION()
	void ShowScoreboard();
	UFUNCTION()
	void HideScoreboard();

	// ���� UI ��ȯ ó��
	void ApplyMatchStateUI_Local(FName State);

protected:
	/** ====================== ���� �帧 / �ð� ����ȭ ====================== */

	virtual void BeginPlay() override;
	void SetHUDTime();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void PollInit();
	void HandleMatchHasStarted();
	virtual void OnUnPossess() override;

	// �������� ��ġ ���� Ȯ��
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// Ŭ�� ���� �ð� ��û
	UFUNCTION(Server, Reliable)
	void ServerRequestimeSync(float TimeOfClientRequest);

	// ������ Ŭ�󿡰� �ð� ����
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// �ֱ��� �ð� ����ȭ
	void CheckTimeSync(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	float ClientServerDelta = 0.f;


	/** ====================== UI ���� / ���� ���� ====================== */

	void EnsureOverlayAndSync(); // Overlay ���� �� Combat �� HUD ����
	void EnsureAlert();          // Alert ���� ����
	void HideAlertIfAny();       // Alert ���� ����

	void StartUIKeepAlive();     // �ֱ��� UI ���� ���� Ÿ�̸� ����
	void UIKeepAliveTick();      // �ֱ������� HUD/Alert ���� ����

public:
	// Ŭ���̾�Ʈ���� Alert ǥ��/����� RPC
	UFUNCTION(Client, Reliable)
	void Client_ShowAlert(const FString& Title);

	UFUNCTION(Client, Reliable)
	void Client_HideAlert();

private:
	FTimerHandle UIKeepAliveTimerHandle;
};
