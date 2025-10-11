// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TFPlayerController.generated.h"

/**
 * 플레이어의 입력, HUD 업데이트, 매치 상태, 네트워크 동기화 등을 관리하는 클래스
 * 클라이언트-서버 간 시간 동기화, UI 표시, 채팅, 점수판 등 다수의 시스템을 통합 제어
 */

UCLASS()
class UNREALPROJECT_7A_API ATFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** ====================== 네트워크 / 매치 상태 관련 ====================== */

	// 현재 매치 상태를 나타냄 (WaitingToStart, InProgress, CoolDown 등)
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	// 매치 상태가 변경되었을 때 호출되는 함수
	UFUNCTION()
	void OnRep_MatchState();

	// 매치 상태가 설정되거나 변경될 때 UI 및 내부 처리 수행
	void OnMatchStateSet(FName State);

	// 서버에서 픽업 효과(사운드/이펙트)를 클라이언트에 재생시킴
	UFUNCTION(Client, Reliable)
	void ClientPlayPickupEffects(USoundCue* Sound, class UNiagaraSystem* Effect, FVector Location, FRotator Rotation);

	// 서버에서 로비의 시작 버튼을 활성화하도록 클라이언트에 알림
	UFUNCTION(Client, Reliable)
	void ClientEnableStartButton();

	// 클라이언트가 버튼을 눌렀을 때 서버에 매치 시작 요청
	UFUNCTION(BlueprintCallable)
	void RequestStartMatch();


	/** ====================== UI / 위젯 관련 ====================== */

	// 로비 위젯 클래스
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ULobbyWidget> LobbyWidgetClass;

	// 로비 위젯 인스턴스
	UPROPERTY()
	class ULobbyWidget* LobbyWidget;

	// 점수판 UI 갱신
	void UpdateScoreboard();

	// 킬피드(죽임 로그)를 클라이언트 HUD에 추가
	UFUNCTION(Client, Reliable)
	void ClientAddKillFeedMessage(const FString& Killer, const FString& Victim);

	// 킬피드 메시지를 HUD에 직접 추가
	void AddKillFeedMessage(const FString& Killer, const FString& Victim);

	// 입력 바인딩 설정
	virtual void SetupInputComponent() override;

	// 메인 메뉴 위젯을 열고 닫음
	void ShowReturnToMainMenu();

	// 메인 메뉴로 돌아가기 위젯 클래스
	UPROPERTY(EditAnywhere, Category = UI)
	TSubclassOf<class UUserWidget> ReturnToMainMenuClass;

	// 메인 메뉴로 돌아가기 위젯 인스턴스
	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	// 플레이어가 킬했을 때 표시되는 위젯
	UFUNCTION(Client, Reliable)
	void ClientShowKillWidget();

	// 플레이어가 죽었을 때 표시되는 위젯
	UFUNCTION(Client, Reliable)
	void ClientShowKilledWidget();

	// 킬 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> KillWidgetClass;

	// 사망 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> KilledWidgetClass;

	// 메인 메뉴가 열려 있는지 여부
	bool bReturnToMainMenuOpen = false;


	/** ====================== HUD 관련 ====================== */

	// 체력 HUD 업데이트
	void SetHUDHealth(float Health, float MaxHealth);

	// 쉴드 HUD 업데이트
	void SetHUDShield(float Shield, float MaxShield);

	// 점수 HUD 업데이트
	void SetHUDScore(float Score);

	// 데스 HUD 업데이트
	void SetHUDDefeats(int32 Defeats);

	// 현재 무기 탄약 HUD 업데이트
	void SetHUDWeaponAmmo(int32 Ammos);

	// 보유 탄약 HUD 업데이트
	void SetHUDCarriedAmmo(int32 Ammos);

	// 매치 카운트다운 HUD 업데이트
	void SetHUDMatchCountdown(float CountdownTime);

	// 알림 카운트다운 HUD 업데이트
	void SetHUDAlertCountDown(float CountdownTime);

	// 수류탄 개수 HUD 업데이트
	void SetHUDGrenadeCount(int32 Grenades);


	/** ====================== 플레이어 상태 / 동기화 ====================== */

	// 플레이어가 Pawn을 소유할 때 호출
	virtual void OnPossess(APawn* InPawn) override;

	// 매 프레임마다 호출
	virtual void Tick(float DeltaTime) override;

	// 클라이언트가 플레이어를 받았을 때 (네트워크 초기화 시)
	virtual void ReceivedPlayer() override;

	// 서버 시간 가져오기 (클라와 동기화된 기준)
	virtual float GetServerTime();

	// 쿨다운 상태 처리
	void HandleCoolDown();

	// 채팅: 서버로 메시지 전송
	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& Message);

	// 채팅: 서버로부터 메시지 수신
	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& Sender, const FString& Message);

	// 히트 사운드 재생
	UFUNCTION(Client, Reliable)
	void ClientPlayHitConfirmSound(class USoundCue* HitSound);

	// 클라이언트가 매치에 참여했을 때 상태 정보 수신
	UFUNCTION(Client, Reliable)
	void ClientJoinMatch(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown);


private:
	/** ====================== 내부 데이터 / 타이머 ====================== */

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

	// HUD 초기화 플래그들
	bool bInitializeHealth = false;
	bool bInitializeShield = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenades = false;
	bool bInitializeAmmos = false;
	bool bInitializeCarriedAmmos = false;

	// HUD 동기화용 임시값들
	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	float HUDDefeats;
	float HUDAmmos;
	float HUDCarriedAmmos;
	int32 HUDGrenades;

	// 점수판 관련
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ScoreboardClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> ScoreboardRowClass;

	UPROPERTY()
	UUserWidget* ScoreboardWidget;

	// 점수판 열기/닫기
	UFUNCTION()
	void ShowScoreboard();
	UFUNCTION()
	void HideScoreboard();

	// 로컬 UI 전환 처리
	void ApplyMatchStateUI_Local(FName State);

protected:
	/** ====================== 게임 흐름 / 시간 동기화 ====================== */

	virtual void BeginPlay() override;
	void SetHUDTime();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void PollInit();
	void HandleMatchHasStarted();
	virtual void OnUnPossess() override;

	// 서버에서 매치 상태 확인
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// 클라가 서버 시간 요청
	UFUNCTION(Server, Reliable)
	void ServerRequestimeSync(float TimeOfClientRequest);

	// 서버가 클라에게 시간 보고
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// 주기적 시간 동기화
	void CheckTimeSync(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	float ClientServerDelta = 0.f;


	/** ====================== UI 유지 / 보정 관련 ====================== */

	void EnsureOverlayAndSync(); // Overlay 보장 및 Combat → HUD 갱신
	void EnsureAlert();          // Alert 위젯 보장
	void HideAlertIfAny();       // Alert 강제 제거

	void StartUIKeepAlive();     // 주기적 UI 상태 점검 타이머 시작
	void UIKeepAliveTick();      // 주기적으로 HUD/Alert 상태 복원

public:
	// 클라이언트에서 Alert 표시/숨김용 RPC
	UFUNCTION(Client, Reliable)
	void Client_ShowAlert(const FString& Title);

	UFUNCTION(Client, Reliable)
	void Client_HideAlert();

private:
	FTimerHandle UIKeepAliveTimerHandle;
};
