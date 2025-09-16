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
	virtual void SetupInputComponent() override; // 입력 컴포넌트를 설정하는 함수
	void ShowReturnToMainMenu(); // 메인 메뉴로 돌아가는 위젯을 표시하는 함수
	UPROPERTY(EditAnywhere, Category = UI)
	TSubclassOf<class UUserWidget> ReturnToMainMenuClass; // 메인 메뉴로 돌아가는 위젯 클래스
	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu; // 메인 메뉴로 돌아가는 위젯 인스턴스
	bool bReturnToMainMenuOpen = false; // 메인 메뉴로 돌아가는 위젯이 열려있는지 여부
	//허드
	void SetHUDHealth(float Health, float MaxHealth); // 허드의 체력을 설정하는 함수
	void SetHUDShield(float Shield, float MaxShield); // 허드의 체력을 설정하는 함수
	void SetHUDScore(float Score); // 허드의 점수를 설정하는 함수
	void SetHUDDefeats(int32 Defeats); // 허드의 처치 수를 설정하는 함수
	void SetHUDWeaponAmmo(int32 Ammos); // 허드의 처치 수를 설정하는 함수
	void SetHUDCarriedAmmo(int32 Ammos); // 허드의 보유 탄약을 설정하는 함수
	void SetHUDMatchCountdown(float CountdownTime); // 허드의 매치 카운트다운을 설정하는 함수 
	void SetHUDAlertCountDown(float CountdownTime); // 허드의 카운트다운을 설정하는 함수
	void SetHUDGrenadeCount(int32 Grenades); // 허드의 수류탄 개수를 설정하는 함수
	virtual void OnPossess(APawn* InPawn) override; // 플레이어가 Pawn을 소유할 때 호출되는 함수
	virtual void Tick(float DeltaTime) override; // 매 프레임마다 호출되는 함수
	virtual void ReceivedPlayer() override; // 플레이어가 컨트롤러를 받았을 때 호출되는 함수
	virtual float GetServerTime(); // 서버 시간을 가져오는 함수
	void OnMatchStateSet(FName State); // 매치 상태가 변경될 때 호출되는 함수
	void HandleCoolDown(); // 쿨다운 상태를 처리하는 함수
	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& Message); //0913 채팅

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& Sender, const FString& Message);//0913 채팅
	UFUNCTION(Client, Reliable)
	void ClientPlayHitConfirmSound(class USoundCue* HitSound); //0914 hitsound
private:
	UPROPERTY()
	class ATFHUD* TfHud; // 메인캐릭터 허드	
	UPROPERTY()
	class ATFGameMode* TFGameMode; // 게임 모드 클래스 포인터
	float LevelStartingTime = 0.f; // 레벨 시작 시간
	uint32 CountdownInt; // 카운트다운 시간 (초 단위)
	float MatchTime = 0.f; // 매치 시간 (초 단위)
	float WarmupTime = 0.f; // 웜업 시간 (초 단위)
	float CoolDownTime = 0.f; // 쿨다운 시간 (초 단위)
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)

	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState(); // 매치 상태가 변경되었을 때 호출되는 함수
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay; // 캐릭터 오버레이 위젯
	bool bInitializeHealth = false; // 허드의 체력이 초기화되었는지 여부
	bool bInitializeShield = false; // 허드의 쉴드가 초기화되었는지 여부
	bool bInitializeScore = false; // 허드의 점수가 초기화되었는지 여부
	bool bInitializeDefeats = false; // 허드의 처치 수가 초기화되었는지 여부
	bool bInitializeGrenades = false; // 허드의 수류탄 개수가 초기화되었는지 여부
	bool bInitializeAmmos = false; // 허드의 탄약이 초기화되었는지 여부
	bool bInitializeCarriedAmmos = false; // 허드의 보유 탄약이 초기화되었는지 여부
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
	virtual void BeginPlay() override; // 플레이어 컨트롤러가 시작될 때 호출되는 함수
	void SetHUDTime(); // HUD의 시간을 설정하는 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//복제하는 항목을 정의하는 함수
	void PollInit(); // 허드와 같은 함수 초기화
	void HandleMatchHasStarted(); // 매치가 시작되었을 때 호출되는 함수
	//
	//클라이언트와 서버 시간 맞추기
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState(); // 서버가 매치 상태를 확인
	UFUNCTION(Client, Reliable)
	void ClientJoinMatch(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown); // 클라이언트가 매치 상태를 확인
	UFUNCTION(Server, Reliable)
	void ServerRequestimeSync(float TimeOfClientRequest); // 클라이언트가 서버의 현재 시간을 요청
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest); // 서버가 클라이언트에게 현재 시간을 보고
	void CheckTimeSync(float DeltaTime); // 시간 동기화 확인 함수
	UPROPERTY(EditAnywhere, Category=Time)
	float TimeSyncFrequency = 5.f; // 시간 동기화 주기 (초 단위)
	float TimeSyncRunningTime = 0.f; // 시간 동기화가 얼마나 진행되었는지 저장하는 변수
	float ClientServerDelta = 0.f; // 클라이언트와 서버 간의 시간 차이를 저장하는 변수


};
