// Fill out your copyright notice in the Description page of Project Settings.

#include "TFPlayerController.h"
#include "UnrealProject_7A/HUD/TFHUD.h"
#include "UnrealProject_7A/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "Net/UnrealNetwork.h"
#include "UnrealProject_7A/GameMode/TFGameMode.h"
#include "UnrealProject_7A/HUD/Alert.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "UnrealProject_7A/GameState/TFGameState.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"
#include "UnrealProject_7A/HUD/ChatWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Sound/SoundCue.h"
#include "UnrealProject_7A/HUD/ReturnToMainMenu.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetTree.h"
#include "UnrealProject_7A/GameMode/LBGameMode.h"
#include "UnrealProject_7A/HUD/LobbyWidget.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

// ============================================================
// 기능: 서버에서 킬피드 메시지를 클라이언트에게 전달.
// 설명: Server → Client RPC로 호출되어 클라이언트에서 HUD 갱신 수행.
// ============================================================
void ATFPlayerController::ClientAddKillFeedMessage_Implementation(const FString& Killer, const FString& Victim)
{
	AddKillFeedMessage(Killer, Victim); // 실제 메시지 추가는 공용 함수에서 처리
}

// ============================================================
// 기능: HUD의 킬피드 UI에 “누가 누구를 죽였는지” 표시.
// 알고리즘:
//   1. 매치가 진행 중(InProgress)일 때만 표시.
//   2. KillFeedBox가 존재하는지 확인 후 새로운 TextBlock 추가.
//   3. 5초 후 자동으로 클리어하는 타이머 설정.
// ============================================================
void ATFPlayerController::AddKillFeedMessage(const FString& Killer, const FString& Victim)
{
	if (MatchState != MatchState::InProgress) return;

	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (!TfHud || !TfHud->CharacterOverlay) return;

	UScrollBox* KillFeedBox = TfHud->CharacterOverlay->KillFeedBox;
	if (!KillFeedBox) return;

	UWidgetTree* WT = TfHud->CharacterOverlay->WidgetTree;
	if (!WT) return;

	UTextBlock* NewMessage = WT->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	const FString Msg = FString::Printf(TEXT("%s killed %s"), *Killer, *Victim);
	NewMessage->SetText(FText::FromString(Msg));
	KillFeedBox->AddChild(NewMessage);

	// KillFeed 자동 클리어 타이머
	TWeakObjectPtr<UScrollBox> WeakBox = KillFeedBox;
	GetWorld()->GetTimerManager().ClearTimer(KillFeedClearTimer);
	GetWorld()->GetTimerManager().SetTimer(
		KillFeedClearTimer,
		[WeakBox]()
		{
			if (WeakBox.IsValid())
			{
				WeakBox.Get()->ClearChildren();
			}
		},
		5.f, false
	);

	if (KillFeedBox->GetChildrenCount() > 5)
	{
		KillFeedBox->RemoveChildAt(0);
	}
}

// ============================================================
// 기능: 키 입력 설정 함수.
// 설명: "Quit"은 메뉴 표시, "Scoreboard"는 점수판 표시/숨김.
// ============================================================
void ATFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &ATFPlayerController::ShowReturnToMainMenu);
	InputComponent->BindAction("Scoreboard", IE_Pressed, this, &ATFPlayerController::ShowScoreboard);
	InputComponent->BindAction("Scoreboard", IE_Released, this, &ATFPlayerController::HideScoreboard);
}

// ============================================================
// 기능: 점수판 표시.
// 알고리즘:
//   1. Scoreboard 위젯이 없으면 생성 후 Viewport에 추가.
//   2. 이미 있더라도 숨겨져 있으면 다시 활성화.
//   3. 표시 후 항상 UpdateScoreboard() 호출로 최신 데이터 갱신.
// ============================================================
void ATFPlayerController::ShowScoreboard()
{
	if (!ScoreboardClass) return;

	if (!IsValid(ScoreboardWidget))
	{
		ScoreboardWidget = CreateWidget<UUserWidget>(this, ScoreboardClass);
		if (ScoreboardWidget) { ScoreboardWidget->AddToViewport(50); }
	}
	else if (!ScoreboardWidget->IsInViewport())
	{
		ScoreboardWidget->AddToViewport(50);
	}

	if (ScoreboardWidget)
	{
		ScoreboardWidget->SetVisibility(ESlateVisibility::Visible);
		UpdateScoreboard();
	}
}

// ============================================================
// 기능: 점수판 숨기기 (Scoreboard 키를 뗐을 때 호출됨)
// ============================================================
void ATFPlayerController::HideScoreboard()
{
	if (ScoreboardWidget)
	{
		ScoreboardWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

// ============================================================
// 기능: ESC/Quit 키 입력 시 메인 메뉴를 표시하거나 닫음.
// 알고리즘:
//   1. 위젯이 없으면 생성.
//   2. 현재 열려있는 상태에 따라 열기(MenuSetup) 또는 닫기(MenuTearDown) 수행.
// ============================================================
void ATFPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuClass == nullptr) return;
	if (ReturnToMainMenu == nullptr) {
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuClass);
	}
	if (ReturnToMainMenu) {
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen) {
			ReturnToMainMenu->MenuSetup();
		}
		else {
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

// ============================================================
// 기능: 클라이언트가 사망했을 때 Killed 위젯을 표시.
// 알고리즘:
//   1. 위젯 생성 후 Viewport에 추가.
//   2. 3초 후 자동 제거.
// ============================================================
void ATFPlayerController::ClientShowKilledWidget_Implementation()
{
	if (KilledWidgetClass == nullptr) return;
	KilledWidgetInstance = CreateWidget<UUserWidget>(this, KilledWidgetClass);
	if (KilledWidgetInstance)
	{
		KilledWidgetInstance->AddToViewport();

		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, [this]()
			{
				if (KilledWidgetInstance) { KilledWidgetInstance->RemoveFromParent(); KilledWidgetInstance = nullptr; }
			}, 3.0f, false);
	}
}

// ============================================================
// 기능: 플레이어가 다른 플레이어를 처치했을 때 Kill 위젯을 표시.
// 알고리즘:
//   1. 위젯 생성 후 화면에 표시.
//   2. 3초 후 자동 제거 타이머 실행.
// ============================================================
void ATFPlayerController::ClientShowKillWidget_Implementation()
{
	if (KillWidgetClass == nullptr) return;
	KillWidgetInstance = CreateWidget<UUserWidget>(this, KillWidgetClass);
	if (KillWidgetInstance)
	{
		KillWidgetInstance->AddToViewport();

		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, [this]()
			{
				if (KillWidgetInstance) { KillWidgetInstance->RemoveFromParent(); KillWidgetInstance = nullptr; }
			}, 3.0f, false);
	}
}
// ============================================================
// 기능: HUD 체력(Health) 표시 업데이트
// 알고리즘:
//   1. TFHUD와 CharacterOverlay의 존재 여부 확인.
//   2. 유효하다면 ProgressBar와 TextBlock에 현재 체력 반영.
//   3. HUD가 아직 생성되지 않았다면 값을 임시 저장 후 나중에 초기화 처리.
// ============================================================
void ATFPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->HealthBar && TfHud->CharacterOverlay->HealthText;

	if (bHUDVaild) {
		const float HealthPercent = Health / MaxHealth;
		TfHud->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		TfHud->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else {
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

// ============================================================
// 기능: HUD 쉴드(Shield) 표시 업데이트
// 알고리즘:
//   1. CharacterOverlay에 Shield 관련 위젯이 존재하면 바로 갱신.
//   2. 존재하지 않으면 HUD 초기화 시점에 적용되도록 값 저장.
// ============================================================
void ATFPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->ShieldBar && TfHud->CharacterOverlay->ShieldText;

	if (bHUDVaild) {
		const float ShieldPercent = Shield / MaxShield;
		TfHud->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		TfHud->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else {
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

// ============================================================
// 기능: HUD 점수(Score) 표시 갱신
// 알고리즘:
//   1. CharacterOverlay의 ScoreAmount 존재 확인.
//   2. HUD가 없을 경우 초기화 예약.
// ============================================================
void ATFPlayerController::SetHUDScore(float Score)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->ScoreAmount;

	if (bHUDVaild) {
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		TfHud->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else {
		bInitializeScore = true;
		HUDScore = Score;
	}
}

// ============================================================
// 기능: HUD의 패배(Defeats) 수 표시 갱신
// 알고리즘:
//   1. CharacterOverlay의 DefeatsAmount가 존재하는지 확인.
//   2. 있으면 바로 표시, 없으면 초기화용 값으로 저장.
// ============================================================
void ATFPlayerController::SetHUDDefeats(int32 Defeats)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->DefeatsAmount;

	if (bHUDVaild) {
		FString DefeatText = FString::Printf(TEXT("%d"), Defeats);
		TfHud->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatText));
	}
	else {
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

// ============================================================
// 기능: 현재 무기 탄약(Weapon Ammo) HUD 표시 갱신
// 알고리즘:
//   1. HUD의 AmmoAmount 텍스트를 갱신.
//   2. HUD가 아직 준비되지 않았다면 값 저장.
// ============================================================
void ATFPlayerController::SetHUDWeaponAmmo(int32 Ammos)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->AmmoAmount;

	if (bHUDVaild) {
		FString AmmoText = FString::Printf(TEXT("%d"), Ammos);
		TfHud->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else {
		bInitializeAmmos = true;
		HUDAmmos = Ammos;
	}
}

// ============================================================
// 기능: 보유 탄약(Carried Ammo) HUD 표시 갱신
// 알고리즘:
//   1. HUD의 CarriedAmmoAmount 텍스트를 갱신.
//   2. HUD 미생성 시 임시 저장 후 PollInit에서 적용.
// ============================================================
void ATFPlayerController::SetHUDCarriedAmmo(int32 Ammos)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDVaild) {
		FString CarriedAmmo = FString::Printf(TEXT("%d"), Ammos);
		TfHud->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmo));
	}
	else {
		bInitializeCarriedAmmos = true;
		HUDCarriedAmmos = Ammos;
	}
}

// ============================================================
// 기능: 매치 시간 카운트다운 표시
// 알고리즘:
//   1. 매치 시간이 0보다 작으면 표시를 지움.
//   2. 초 단위 시간을 분:초 형태로 포맷팅 후 텍스트 갱신.
// ============================================================
void ATFPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->MatchCountDownText;

	if (bHUDVaild) {
		if (CountdownTime < 0.f) {
			TfHud->CharacterOverlay->MatchCountDownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TfHud->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}

// ============================================================
// 기능: 알림(Alert) 위젯의 카운트다운 텍스트 갱신
// 알고리즘:
//   1. TfHud.Alert 위젯의 WarmupTime 텍스트 존재 여부 확인.
//   2. 유효하면 분:초 형식으로 표시.
// ============================================================
void ATFPlayerController::SetHUDAlertCountDown(float CountdownTime)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	bool bHUDVaild = TfHud && TfHud->Alert && TfHud->Alert->WarmupTime;

	if (bHUDVaild) {
		if (CountdownTime < 0.f) {
			TfHud->Alert->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - (Minutes * 60);
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TfHud->Alert->WarmupTime->SetText(FText::FromString(CountDownText));
	}
}

// ============================================================
// 기능: HUD 수류탄 개수 표시 갱신
// 알고리즘:
//   1. HUD가 존재하면 GrenadeAmount 텍스트에 표시.
//   2. HUD가 아직 없으면 나중에 초기화 시 반영.
// ============================================================
void ATFPlayerController::SetHUDGrenadeCount(int32 Grenades)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->GrenadeAmount;

	if (bHUDVaild) {
		FString GrenadeText = FString::Printf(TEXT("%d"), Grenades);
		TfHud->CharacterOverlay->GrenadeAmount->SetText(FText::FromString(GrenadeText));
	}
	else {
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}
// ============================================================
// 기능: 플레이어가 Pawn을 소유할 때 HUD 및 입력 설정.
// 알고리즘:
//   1. HUD 캐시 갱신 및 PollInit 타이머 활성화.
//   2. 소유 중인 Pawn(캐릭터)의 체력, 쉴드 HUD 즉시 반영.
//   3. 마우스 커서 비활성화 및 게임 전용 입력 모드 설정.
// ============================================================
void ATFPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	TfHud = Cast<ATFHUD>(GetHUD());
	GetWorldTimerManager().ClearTimer(PollInitTimerHandle);
	GetWorldTimerManager().SetTimer(PollInitTimerHandle, this, &ATFPlayerController::PollInit, 0.2f, true);

	if (ATimeFractureCharacter* TfCharacter = Cast<ATimeFractureCharacter>(InPawn))
	{
		SetHUDHealth(TfCharacter->GetHealth(), TfCharacter->GetMaxHealth());
		SetHUDShield(TfCharacter->GetShield(), TfCharacter->GetMaxShield());
	}

	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	StartUIKeepAlive(); // UI 유지 타이머 시작
}

// ============================================================
// 기능: HUD의 시간 텍스트를 갱신.
// 알고리즘:
//   1. 서버일 경우 GameMode에서 직접 카운트다운 값을 얻음.
//   2. 클라일 경우 서버 시간 기준으로 로컬에서 계산.
//   3. 상태별(대기, 진행, 쿨다운)에 따라 알맞은 HUD 영역에 표시.
// ============================================================
void ATFPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;

	if (HasAuthority())
	{
		TFGameMode = TFGameMode == nullptr ? Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)) : TFGameMode;
		if (TFGameMode)
		{
			TimeLeft = TFGameMode->GetCountdownTime();
		}
	}
	else
	{
		if (MatchState == MatchState::WaitingToStart)
			TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
		else if (MatchState == MatchState::InProgress)
			TimeLeft = MatchTime - GetServerTime() + LevelStartingTime;
		else if (MatchState == MatchState::CoolDown)
			TimeLeft = CoolDownTime - GetServerTime() + LevelStartingTime;
	}

	if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::CoolDown)
		SetHUDAlertCountDown(TimeLeft);
	else if (MatchState == MatchState::InProgress)
		SetHUDMatchCountdown(TimeLeft);

	if (MatchState == MatchState::InProgress)
	{
		HideAlertIfAny();
	}
}

// ============================================================
// 기능: 서버와 클라이언트 간 시간 차이 보정 알고리즘.
// 알고리즘:
//   1. 매 프레임마다 누적 시간 증가.
//   2. 일정 주기(TimeSyncFrequency)마다 서버에 현재 시간을 요청.
//   3. 서버 응답을 받아 ClientServerDelta 보정값 계산.
// ============================================================
void ATFPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;

	if (IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency)
	{
		ServerRequestimeSync(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

// ============================================================
// 기능: 서버로부터 클라이언트에게 시간 정보를 보고받음.
// 알고리즘:
//   1. 클라가 요청을 보낸 시간(TimeOfClientRequest)과
//      서버가 받은 시간(TimeServerReceivedClientRequest)을 이용해 왕복 시간 계산.
//   2. 클라-서버 시간차(ClientServerDelta) 보정값 설정.
// ============================================================
void ATFPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float ServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);
	ClientServerDelta = ServerTime - GetWorld()->GetTimeSeconds();
}

// ============================================================
// 기능: 클라이언트 → 서버 시간 요청.
// 알고리즘:
//   1. 클라이언트가 자신의 요청 시간을 전달.
//   2. 서버가 요청을 받은 즉시 현재 서버 시간을 포함해 응답 RPC 전송.
// ============================================================
void ATFPlayerController::ServerRequestimeSync_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

// ============================================================
// 기능: 플레이어가 컨트롤러를 받을 때 실행 (네트워크 초기화 이후 호출됨).
// 설명: 로컬 컨트롤러일 경우 즉시 서버에 시간 동기화 요청을 보냄.
// ============================================================
void ATFPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestimeSync(GetWorld()->GetTimeSeconds());
	}
}

// ============================================================
// 기능: 서버 시간 계산 (클라이언트 보정 포함).
// 알고리즘:
//   - 서버는 자신의 World Time을 그대로 사용.
//   - 클라는 ClientServerDelta(보정값)를 더해 서버 시간에 근접시킴.
// ============================================================
float ATFPlayerController::GetServerTime()
{
	if (HasAuthority())
		return GetWorld()->GetTimeSeconds();
	else
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

// ============================================================
// 기능: MatchState가 복제될 때 호출되는 함수.
// 설명: OnRep_ 함수는 클라이언트에서 복제 값이 바뀌었을 때 트리거됨.
// ============================================================
void ATFPlayerController::OnRep_MatchState()
{
	OnMatchStateSet(MatchState);
}

// ============================================================
// 기능: 매치 상태 변경 시 UI 및 내부 처리 수행.
// 알고리즘:
//   1. MatchState 값 저장.
//   2. ApplyMatchStateUI_Local()로 HUD/Alert 상태 갱신.
// ============================================================
void ATFPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	ApplyMatchStateUI_Local(MatchState);
}
// ============================================================
// 기능: 매치가 시작될 때 호출.
// 알고리즘:
//   1. 서버면 모든 클라이언트에게 Alert 제거 명령 전송(Client_HideAlert).
//   2. 클라면 직접 Alert 위젯 제거.
//   3. 오버레이(체력/탄약 표시 UI)가 없으면 강제로 생성 및 HUD 동기화.
// ============================================================
void ATFPlayerController::HandleMatchHasStarted() {
	if (HasAuthority())
		Client_HideAlert();
	else
		HideAlertIfAny();

	EnsureOverlayAndSync();
}

// ============================================================
// 기능: 플레이어가 Pawn을 잃었을 때(죽거나 라운드 종료 시) 호출.
// 알고리즘:
//   1. HUD 포인터 재확인 및 타이머 정리.
//   2. 쿨다운 상태에서는 오버레이 제거 후 Alert 강제 표시.
// ============================================================
void ATFPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	GetWorldTimerManager().ClearTimer(KillFeedClearTimer);

	if (MatchState == MatchState::CoolDown)
	{
		if (TfHud && TfHud->CharacterOverlay)
		{
			TfHud->CharacterOverlay->RemoveFromParent();
			TfHud->CharacterOverlay = nullptr;
		}
		EnsureAlert();
	}
}

// ============================================================
// 기능: 쿨다운 상태(게임 종료 후 승자 표시 및 대기).
// 알고리즘:
//   1. 킬피드 제거 및 HUD 오버레이 숨김.
//   2. Alert 위젯 생성 후 “Winner” 텍스트 구성.
//   3. GameState의 TopScorePlayers 기준으로 승자 판단.
//   4. 클라 조작 비활성화로 오발 방지.
// ============================================================
void ATFPlayerController::HandleCoolDown()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (!TfHud) return;

	GetWorldTimerManager().ClearTimer(KillFeedClearTimer);

	if (TfHud->CharacterOverlay)
	{
		TfHud->CharacterOverlay->RemoveFromParent();
		TfHud->CharacterOverlay = nullptr;
	}

	EnsureAlert(); // 쿨다운 알림 보장

	if (TfHud->Alert)
	{
		TfHud->Alert->SetVisibility(ESlateVisibility::Visible);
		TfHud->Alert->AlertText->SetText(FText::FromString(TEXT("New Match Starting In:")));

		if (ATFGameState* GS = Cast<ATFGameState>(UGameplayStatics::GetGameState(this)))
		{
			TArray<ATFPlayerState*> TopPlayers = GS->TopScorePlayers;

			// 보조 로직: TopScorePlayers가 비었을 경우 직접 최고 점수 탐색
			if (TopPlayers.Num() == 0)
			{
				float TopScore = TNumericLimits<float>::Lowest();
				for (APlayerState* APS : GS->PlayerArray)
				{
					if (ATFPlayerState* TPS = Cast<ATFPlayerState>(APS))
					{
						if (TPS->GetScore() > TopScore)
						{
							TopScore = TPS->GetScore();
						}
					}
				}
				for (APlayerState* APS : GS->PlayerArray)
				{
					if (ATFPlayerState* TPS = Cast<ATFPlayerState>(APS))
					{
						if (FMath::IsNearlyEqual(TPS->GetScore(), TopScore))
						{
							TopPlayers.Add(TPS);
						}
					}
				}
			}

			// 표시 문구 구성
			if (ATFPlayerState* PS = GetPlayerState<ATFPlayerState>())
			{
				FString Info;
				if (TopPlayers.Num() == 0)
				{
					Info = TEXT("No Winner");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == PS)
				{
					Info = TEXT("You are the Winner");
				}
				else if (TopPlayers.Num() == 1)
				{
					Info = FString::Printf(TEXT("Winner: %s"), *TopPlayers[0]->GetPlayerName());
				}
				else
				{
					Info = TEXT("Players tied for the win:\n");
					for (ATFPlayerState* P : TopPlayers)
					{
						Info.Append(P->GetPlayerName()).Append(TEXT("\n"));
					}
				}
				TfHud->Alert->InfoText->SetText(FText::FromString(Info));
			}
		}
	}

	// 조작 비활성화 (라운드 종료 시점)
	if (ATimeFractureCharacter* MyChar = Cast<ATimeFractureCharacter>(GetPawn()))
	{
		if (MyChar->GetCombatComponent())
		{
			MyChar->bDisableGameplay = true;
			MyChar->GetCombatComponent()->FireButtonPressed(false);
		}
	}
}

// ============================================================
// 기능: 클라이언트가 서버로부터 매치 정보를 수신할 때 실행.
// 알고리즘:
//   1. 매치 상태, 시간, 웜업, 쿨다운, 레벨 시작시간 등을 저장.
//   2. ApplyMatchStateUI_Local() 호출하여 UI를 해당 상태에 맞게 갱신.
// ============================================================
void ATFPlayerController::ClientJoinMatch_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown)
{
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	CoolDownTime = CoolDown;

	ApplyMatchStateUI_Local(MatchState);
}

// ============================================================
// 기능: 로컬 UI를 현재 매치 상태에 맞게 전환.
// 알고리즘:
//   1. 대기(WaitingToStart): Alert 생성 및 표시 (“Match Starting In…”).
//   2. 진행(InProgress): Alert 제거 → Overlay 생성 및 HUD 동기화.
//   3. 쿨다운(CoolDown): HandleCoolDown 호출하여 승자 표시 및 대기.
// ============================================================
void ATFPlayerController::ApplyMatchStateUI_Local(FName State)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;

	if (State == MatchState::WaitingToStart)
	{
		EnsureAlert();
		if (TfHud && TfHud->Alert)
		{
			TfHud->Alert->SetVisibility(ESlateVisibility::Visible);
			TfHud->Alert->AlertText->SetText(FText::FromString(TEXT("Match Starting In:")));
		}
	}
	else if (State == MatchState::InProgress)
	{
		HideAlertIfAny();
		EnsureOverlayAndSync();

		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, [this]()
			{
				HideAlertIfAny();
			}, 0.1f, false);
	}
	else if (State == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

// ============================================================
// 기능: 서버가 현재 매치 상태를 클라이언트에게 동기화.
// 알고리즘:
//   1. GameMode에서 시간 관련 데이터(Warmup, Match, Cooldown 등) 획득.
//   2. 클라이언트 RPC(ClientJoinMatch)로 해당 값 전달.
// ============================================================
void ATFPlayerController::ServerCheckMatchState_Implementation()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	ATFGameMode* GameMode = Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this));

	if (GameMode)
	{
		CoolDownTime = GameMode->CoolDownTime;
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();

		ClientJoinMatch(MatchState, WarmupTime, MatchTime, LevelStartingTime, CoolDownTime);
	}
}
// ============================================================
// 기능: HUD와 각종 변수 초기화를 폴링(Polling) 방식으로 보장.
// 알고리즘:
//   1. HUD 및 CharacterOverlay가 생성될 때까지 주기적으로 검사.
//   2. bInitializeXXX 값이 true인 항목들을 실제 HUD에 반영.
//   3. 매치 상태가 None이면 서버에 상태 동기화 요청.
// ============================================================
void ATFPlayerController::PollInit() {
	if (!TfHud) TfHud = Cast<ATFHUD>(GetHUD());
	if (!TfHud || !TfHud->CharacterOverlay) return;

	CharacterOverlay = TfHud->CharacterOverlay;

	if (bInitializeHealth) { SetHUDHealth(HUDHealth, HUDMaxHealth); bInitializeHealth = false; }
	if (bInitializeShield) { SetHUDShield(HUDShield, HUDMaxShield); bInitializeShield = false; }
	if (bInitializeScore) { SetHUDScore(HUDScore); bInitializeScore = false; }
	if (bInitializeDefeats) { SetHUDDefeats(HUDDefeats); bInitializeDefeats = false; }
	if (bInitializeAmmos) { SetHUDWeaponAmmo(HUDAmmos); bInitializeAmmos = false; }
	if (bInitializeCarriedAmmos) { SetHUDCarriedAmmo(HUDCarriedAmmos); bInitializeCarriedAmmos = false; }

	if (ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(GetPawn()))
	{
		if (TFCharacter->GetCombatComponent() && bInitializeGrenades)
		{
			SetHUDGrenadeCount(TFCharacter->GetCombatComponent()->GetGrenades());
			bInitializeGrenades = false;
		}
	}

	if (!TfHud->Alert && MatchState == MatchState::WaitingToStart)
	{
		TfHud->AddAlert();
	}

	if (MatchState == NAME_None) {
		ServerCheckMatchState();
	}

	if (CharacterOverlay)
	{
		GetWorldTimerManager().ClearTimer(PollInitTimerHandle);
	}
}

// ============================================================
// 기능: 서버에서 채팅 메시지 브로드캐스트.
// 알고리즘:
//   1. 메시지를 전송한 클라이언트의 PlayerState에서 이름 추출.
//   2. 현재 접속 중인 모든 PlayerController 순회.
//   3. 각 클라이언트에 ClientReceiveChatMessage RPC 호출.
// ============================================================
void ATFPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	APlayerState* PS = GetPlayerState<APlayerState>();
	FString SenderName = PS ? PS->GetPlayerName() : TEXT("Unknown");

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATFPlayerController* PC = Cast<ATFPlayerController>(*It);
		if (PC)
		{
			PC->ClientReceiveChatMessage(SenderName, Message);
		}
	}
}

// ============================================================
// 기능: 클라이언트에서 채팅 메시지를 수신하고 HUD에 표시.
// 알고리즘:
//   1. HUD 및 ChatWidget 존재 여부 확인.
//   2. ChatWidget이 없으면 즉시 생성(AddChatWidget).
//   3. AddChatMessage로 이름 + 내용 추가 후 표시 유지.
// ============================================================
void ATFPlayerController::ClientReceiveChatMessage_Implementation(const FString& Sender, const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("ClientReceiveChatMessage: %s : %s"), *Sender, *Message);

	ATFHUD* Hud = Cast<ATFHUD>(GetHUD());
	if (!Hud) { UE_LOG(LogTemp, Warning, TEXT("HUD is NULL")); return; }

	if (!Hud->ChatWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChatWidget is NULL, creating..."));
		Hud->AddChatWidget();
	}

	if (Hud->ChatWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddChatMessage EXCUTE!"));
		Hud->ChatWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Hud->ChatWidget->AddChatMessage(Sender, Message);
	}
}

// ============================================================
// 기능: 로비 호스트에게 “시작 버튼” 표시 신호를 전달.
// 알고리즘:
//   1. LobbyWidget이 아직 생성되지 않은 경우 약간 지연 후 재시도.
//   2. LobbyWidget이 유효하면 버튼 표시 및 입력모드 전환.
// ============================================================
void ATFPlayerController::ClientEnableStartButton_Implementation()
{
	if (!LobbyWidget)
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [this]()
			{
				if (LobbyWidget)
				{
					LobbyWidget->ShowStartButton();
					bShowMouseCursor = true;

					FInputModeGameAndUI InputMode;
					InputMode.SetWidgetToFocus(LobbyWidget->TakeWidget());
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					SetInputMode(InputMode);
				}
			}, 0.2f, false);
		return;
	}

	LobbyWidget->ShowStartButton();
	bShowMouseCursor = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(LobbyWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

// ============================================================
// 기능: 서버에서 픽업(아이템 습득) 효과를 클라이언트 전역 재생.
// 알고리즘:
//   1. 사운드가 존재하면 해당 위치에서 재생.
//   2. Niagara 시스템(파티클 이펙트)이 지정되어 있으면 스폰.
// ============================================================
void ATFPlayerController::ClientPlayPickupEffects_Implementation(USoundCue* Sound, UNiagaraSystem* Effect, FVector Location, FRotator Rotation)
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
	}
	if (Effect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Effect, Location, Rotation);
	}
}

// ============================================================
// 기능: 클라이언트가 호스트일 경우 게임 시작 요청.
// 알고리즘:
//   1. 현재 GameMode를 LBGameMode로 캐스팅.
//   2. 서버 권한이 있을 때만 ServerStartMatch() 실행.
// ============================================================
void ATFPlayerController::RequestStartMatch()
{
	if (HasAuthority())
	{
		ALBGameMode* LobbyGM = Cast<ALBGameMode>(UGameplayStatics::GetGameMode(this));
		if (LobbyGM)
		{
			LobbyGM->ServerStartMatch();
		}
	}
}

// ============================================================
// 기능: 점수판(Scoreboard) UI 갱신.
// 알고리즘:
//   1. Scoreboard 위젯의 ScrollBox(PlayerListBox) 탐색.
//   2. 현재 GameState의 모든 PlayerState를 순회.
//   3. 이름 / 킬수 / 데스수 텍스트를 Row 위젯에 채워넣음.
// ============================================================
void ATFPlayerController::UpdateScoreboard()
{
	if (!ScoreboardWidget) return;

	UScrollBox* PlayerListBox = Cast<UScrollBox>(ScoreboardWidget->GetWidgetFromName(TEXT("PlayerListBox")));
	if (!PlayerListBox) return;

	PlayerListBox->ClearChildren();

	ATFGameState* GS = Cast<ATFGameState>(UGameplayStatics::GetGameState(this));
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
	{
		ATFPlayerState* TFPS = Cast<ATFPlayerState>(PS);
		if (TFPS && ScoreboardRowClass)
		{
			UUserWidget* Row = CreateWidget<UUserWidget>(this, ScoreboardRowClass);
			if (!Row) continue;

			UTextBlock* NameText = Cast<UTextBlock>(Row->GetWidgetFromName(TEXT("NameText")));
			UTextBlock* KillsText = Cast<UTextBlock>(Row->GetWidgetFromName(TEXT("KillsText")));
			UTextBlock* DeathsText = Cast<UTextBlock>(Row->GetWidgetFromName(TEXT("DeathsText")));

			if (NameText) {
				FString DisplayString = TFPS->GetPlayerName() + TEXT(" / ");
				NameText->SetText(FText::FromString(DisplayString));
			}
			if (KillsText) {
				FString DisplayString = FString::FromInt(FMath::FloorToInt(TFPS->GetScore())) + TEXT(" / ");
				KillsText->SetText(FText::FromString(DisplayString));
			}
			if (DeathsText)
			{
				DeathsText->SetText(FText::AsNumber(TFPS->GetDefeats()));
			}

			PlayerListBox->AddChild(Row);
		}
	}
}
// ============================================================
// 기능: 플레이어 컨트롤러의 BeginPlay (로컬 초기화).
// 알고리즘:
//   1. 서버에게 매치 상태 정보 요청(ServerCheckMatchState).
//   2. 로비 상태일 경우 LobbyWidget 생성 후 비활성화 버튼 숨김.
//   3. 인게임에서는 마우스커서 및 입력 모드 전환.
//   4. HUD가 준비될 때까지 PollInit 타이머로 반복 검사.
//   5. UI 유지용 주기 타이머(UIKeepAlive) 시작.
// ============================================================
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ServerCheckMatchState();

	if (IsLocalController() && LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
			LobbyWidget->HideStartButton();
		}
	}

	if (HasAuthority() && GetWorld()->GetAuthGameMode()->GetClass()->GetName().Contains("TFGameMode"))
	{
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}

	GetWorldTimerManager().SetTimer(PollInitTimerHandle, this, &ATFPlayerController::PollInit, 0.2f, true);
	StartUIKeepAlive();
}

// ============================================================
// 기능: 매 프레임마다 시간·HUD 동기화 수행.
// 알고리즘:
//   1. HUD 카운트다운 텍스트 갱신(SetHUDTime).
//   2. 클라-서버 시간 동기화(CheckTimeSync).
//   3. Overlay가 사라졌을 경우 재생성.
// ============================================================
void ATFPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);

	if (!CharacterOverlay && MatchState == MatchState::InProgress)
	{
		PollInit();
		EnsureOverlayAndSync();
	}
}

// ============================================================
// 기능: 네트워크 복제 변수 등록.
// 설명: MatchState가 클라이언트로 복제되도록 설정.
// ============================================================
void ATFPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATFPlayerController, MatchState);
}

// ============================================================
// 기능: UI 자동 보정 주기 타이머 시작.
// 알고리즘:
//   - 일정 주기(1.5초)마다 UIKeepAliveTick 실행.
// ============================================================
void ATFPlayerController::StartUIKeepAlive()
{
	GetWorldTimerManager().SetTimer(UIKeepAliveTimerHandle, this, &ATFPlayerController::UIKeepAliveTick, 1.5f, true);
}

// ============================================================
// 기능: UI 유지 알고리즘.
// 알고리즘:
//   1. 매치가 진행 중일 때만 동작.
//   2. Overlay가 없으면 생성 및 HUD 데이터 강제 푸시.
//   3. Alert 위젯이 남아 있으면 강제 제거 (시야 중첩 방지).
// ============================================================
void ATFPlayerController::UIKeepAliveTick()
{
	if (MatchState == MatchState::InProgress)
	{
		EnsureOverlayAndSync();
		HideAlertIfAny();
	}
}

// ============================================================
// 기능: Overlay 위젯 존재 보장 + HUD 강제 동기화.
// 알고리즘:
//   1. HUD 캐시 갱신 및 CharacterOverlay 확인.
//   2. Overlay가 없으면 AddCharacterOverlay()로 생성.
//   3. Pawn의 CombatComponent에서 PushAllHUDFromCombat() 호출.
// ============================================================
void ATFPlayerController::EnsureOverlayAndSync()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (!TfHud) return;

	if (!TfHud->CharacterOverlay)
	{
		TfHud->AddCharacterOverlay();
	}

	CharacterOverlay = TfHud->CharacterOverlay;

	if (APawn* P = GetPawn())
	{
		if (ATimeFractureCharacter* C = Cast<ATimeFractureCharacter>(P))
		{
			if (UCBComponent* CB = C->GetCombatComponent())
			{
				CB->PushAllHUDFromCombat();
			}
		}
	}
}

// ============================================================
// 기능: Alert 위젯 존재 보장.
// 알고리즘:
//   1. HUD 캐시 갱신.
//   2. Alert 위젯이 없으면 AddAlert()으로 즉시 생성.
// ============================================================
void ATFPlayerController::EnsureAlert()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (TfHud && !TfHud->Alert)
	{
		TfHud->AddAlert();
	}
}

// ============================================================
// 기능: Alert 위젯 강제 제거.
// 알고리즘:
//   1. 현재 HUD 포인터에서 Alert 제거 및 NULL 처리.
//   2. HUD 클래스 기반으로 남은 위젯도 전부 RemoveFromParent.
// ============================================================
void ATFPlayerController::HideAlertIfAny()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (!TfHud) return;

	if (TfHud->Alert)
	{
		TfHud->Alert->SetVisibility(ESlateVisibility::Hidden);
		TfHud->Alert->RemoveFromParent();
		TfHud->Alert = nullptr;
	}

	if (TfHud->AlertClass)
	{
		TArray<UUserWidget*> Existing;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Existing, TfHud->AlertClass, false);
		for (UUserWidget* W : Existing)
		{
			if (W) W->RemoveFromParent();
		}
	}
}

// ============================================================
// 기능: 클라이언트에서 Alert 위젯을 명시적으로 표시.
// 알고리즘:
//   1. Alert 위젯이 없으면 AddAlert() 호출.
//   2. 표시 후 제목 텍스트를 인자로부터 설정.
// ============================================================
void ATFPlayerController::Client_ShowAlert_Implementation(const FString& Title)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (!TfHud) return;

	if (!TfHud->Alert) TfHud->AddAlert();
	if (TfHud->Alert)
	{
		TfHud->Alert->SetVisibility(ESlateVisibility::Visible);
		TfHud->Alert->AlertText->SetText(FText::FromString(Title));
	}
}
void ATFPlayerController::ClientPlayHitConfirmSound_Implementation(USoundCue* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}
// ============================================================
// 기능: 클라이언트에서 Alert 숨기기.
// 알고리즘:
//   - HideAlertIfAny() 호출하여 Alert 관련 위젯 전체 제거.
// ============================================================
void ATFPlayerController::Client_HideAlert_Implementation()
{
	HideAlertIfAny();
}


