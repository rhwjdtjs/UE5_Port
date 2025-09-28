// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerController.h"
#include "UnrealProject_7A/HUD/TFHUD.h" // TFHUD 헤더파일을 포함시킨다.
#include "UnrealProject_7A/HUD/CharacterOverlay.h" 
#include "Components/ProgressBar.h" // UProgressBar 헤더 파일 포함
#include "Components/TextBlock.h" // UTextBlock 헤더 파일 포함
#include "UnrealProject_7A/Character/TimeFractureCharacter.h" // TimeFractureCharacter 헤더파일을 포함시킨다.
#include "UnrealProject_7A/Weapon/Weapon.h" // Weapon 헤더파일을 포함시킨다.
#include "Net/UnrealNetwork.h" // 네트워크 관련 헤더 파일 포함
#include "UnrealProject_7A/GameMode/TFGameMode.h" // TFGameMode 헤더파일을 포함시킨다.
#include "UnrealProject_7A/HUD/Alert.h" // Alert 헤더파일을 포함시킨다.
#include "Kismet/GameplayStatics.h" // 게임플레이 스태틱스 헤더 파일 포함
#include "UnrealProject_7A/TFComponents/CBComponent.h" // CBComponent 헤더파일을 포함시킨다.
#include "UnrealProject_7A/GameState/TFGameState.h" // TFPlayerState 헤더파일을 포함시킨다.
#include "UnrealProject_7A/PlayerState/TFPlayerState.h" // TFPlayerState 헤더파일을 포함시킨다.
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
#include "Blueprint/WidgetBlueprintLibrary.h" // Alert/Overlay 위젯을 안전하게 강제 제거할 때 사용


void ATFPlayerController::ClientAddKillFeedMessage_Implementation(const FString& Killer, const FString& Victim)
{
	AddKillFeedMessage(Killer, Victim); // 클라에서 실제 메시지 추가 실행
}
void ATFPlayerController::AddKillFeedMessage(const FString& Killer, const FString& Victim)
{
	// InProgress 상태에서만 표시
	if (MatchState != MatchState::InProgress) return;

	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (!TfHud) return;

	// Overlay가 없으면 생성하지 않고 리턴 (겹침 방지)
	if (!TfHud->CharacterOverlay) return;

	UScrollBox* KillFeedBox = TfHud->CharacterOverlay->KillFeedBox;
	if (!KillFeedBox) return;

	UWidgetTree* WT = TfHud->CharacterOverlay->WidgetTree;
	if (!WT) return;

	UTextBlock* NewMessage = WT->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	const FString Msg = FString::Printf(TEXT("%s killed %s"), *Killer, *Victim);
	NewMessage->SetText(FText::FromString(Msg));
	KillFeedBox->AddChild(NewMessage);


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
void ATFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent(); // 부모 클래스의 SetupInputComponent 호출
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &ATFPlayerController::ShowReturnToMainMenu); // "Quit" 액션이 눌렸을 때 ShowReturnToMainMenu 함수 호출
	InputComponent->BindAction("Scoreboard", IE_Pressed, this, &ATFPlayerController::ShowScoreboard);
	InputComponent->BindAction("Scoreboard", IE_Released, this, &ATFPlayerController::HideScoreboard);
}
void ATFPlayerController::ShowScoreboard()
{
	if (!ScoreboardClass) return;

	// 재시작/트래블 후: 포인터는 살아있어도 위젯이 죽었거나 뷰포트에서 빠져있을 수 있음
	if (!IsValid(ScoreboardWidget))
	{
		ScoreboardWidget = CreateWidget<UUserWidget>(this, ScoreboardClass);
		if (ScoreboardWidget) { ScoreboardWidget->AddToViewport(50); }
	}
	else if (!ScoreboardWidget->IsInViewport())
	{
		// 뷰포트에 없으면 다시 얹기
		ScoreboardWidget->AddToViewport(50);
	}

	if (ScoreboardWidget)
	{
		ScoreboardWidget->SetVisibility(ESlateVisibility::Visible);
		UpdateScoreboard(); // 켤 때 항상 갱신
	}
}

void ATFPlayerController::HideScoreboard()
{
	if (ScoreboardWidget)
	{
		ScoreboardWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
void ATFPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuClass == nullptr) return;
	if(ReturnToMainMenu == nullptr) {
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuClass); // ReturnToMainMenu 위젯 생성
	}
	if (ReturnToMainMenu) {
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen) {
			ReturnToMainMenu->MenuSetup(); // 메뉴 설정
		}
		else {
			ReturnToMainMenu->MenuTearDown(); // 메뉴 해제
		}
	}

}
void ATFPlayerController::ClientShowKilledWidget_Implementation()
{
	if (KilledWidgetClass == nullptr) return;
	KilledWidgetInstance = CreateWidget<UUserWidget>(this, KilledWidgetClass);
	if (KilledWidgetInstance)
	{
		KilledWidgetInstance->AddToViewport();
		// 필요하면 타이머로 자동 제거
		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, [this]()
			{
				if (KilledWidgetInstance) { KilledWidgetInstance->RemoveFromParent(); KilledWidgetInstance = nullptr; }
			}, 3.0f, false); // 3초 후 제거
	}
}
void ATFPlayerController::ClientShowKillWidget_Implementation()
{
	if (KillWidgetClass == nullptr) return;
	KillWidgetInstance = CreateWidget<UUserWidget>(this, KillWidgetClass);
	if (KillWidgetInstance)
	{
		KillWidgetInstance->AddToViewport();
		// 필요하면 타이머로 자동 제거
		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, [this]()
			{
				if (KillWidgetInstance) { KillWidgetInstance->RemoveFromParent(); KillWidgetInstance = nullptr; }
			}, 3.0f, false); // 3초 후 제거
	}
}
void ATFPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->HealthBar && TfHud->CharacterOverlay->HealthText;
	if (bHUDVaild) {
		const float HealthPercent = Health / MaxHealth; // 체력 비율을 계산한다.
		TfHud->CharacterOverlay->HealthBar->SetPercent(HealthPercent); // 체력바의 비율을 설정한다.
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth)); // 체력 텍스트를 포맷팅한다.
		TfHud->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText)); // 체력 텍스트를 설정한다.
	}
	else {
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth; // HUDHealth와 HUDMaxHealth를 초기화한다.
	}
}
void ATFPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->ShieldBar && TfHud->CharacterOverlay->ShieldText;
	if (bHUDVaild) {
		const float ShieldPercent = Shield / MaxShield; // 체력 비율을 계산한다.
		TfHud->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent); // 체력바의 비율을 설정한다.
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield)); // 체력 텍스트를 포맷팅한다.
		TfHud->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText)); // 체력 텍스트를 설정한다.
	}
	else {
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield; // HUDHealth와 HUDMaxHealth를 초기화한다.
	}
}

void ATFPlayerController::SetHUDScore(float Score)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->ScoreAmount;
	if(bHUDVaild) {
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score)); // 점수 텍스트를 포맷팅한다.
		TfHud->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else {
		bInitializeScore = true;
		HUDScore = Score; // HUDScore를 초기화한다.
	}
}

void ATFPlayerController::SetHUDDefeats(int32 Defeats)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->DefeatsAmount;
	if (bHUDVaild) {
		FString DefeatText = FString::Printf(TEXT("%d"), Defeats); // 점수 텍스트를 포맷팅한다.
		TfHud->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatText));
	}
	else {
		bInitializeDefeats = true;
		HUDDefeats = Defeats; // HUDDefeats를 초기화한다.
	}
}

void ATFPlayerController::SetHUDWeaponAmmo(int32 Ammos)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
		bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->AmmoAmount;
		if (bHUDVaild) {
			FString AmmoText = FString::Printf(TEXT("%d"), Ammos); // 현재 탄약과 최대 탄약을 포맷팅한다.
			TfHud->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
		}
		else {
			bInitializeAmmos = true;
			HUDAmmos = Ammos; // HUDAmmos를 초기화한다.
		}
}
void ATFPlayerController::SetHUDCarriedAmmo(int32 Ammos)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDVaild) {
		FString CarriedAmmo = FString::Printf(TEXT("%d"), Ammos); // 현재 탄약과 최대 탄약을 포맷팅한다.
		TfHud->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmo));
	}
	else {
		bInitializeCarriedAmmos = true;
		HUDCarriedAmmos = Ammos; // HUDCarriedAmmos를 초기화한다.
	}
}
void ATFPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->MatchCountDownText;
	if (bHUDVaild) {
		if (CountdownTime < 0.f) {
			TfHud->CharacterOverlay->MatchCountDownText->SetText(FText());
			return; // 카운트다운 시간이 0보다 작으면 텍스트를 비운다.
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f); // 카운트다운 시간을 분 단위로 변환한다.
		int32 Seconds = CountdownTime - (Minutes * 60); // 카운트다운 시간을 초 단위로 변환한다.
		FString CountDownText=FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		TfHud->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}
void ATFPlayerController::SetHUDAlertCountDown(float CountdownTime)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->Alert && TfHud->Alert->WarmupTime;
	if (bHUDVaild) {
		if (CountdownTime < 0.f) {
			TfHud->Alert->WarmupTime->SetText(FText());
			return; // 카운트다운 시간이 0보다 작으면 텍스트를 비운다.
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f); // 카운트다운 시간을 분 단위로 변환한다.
		int32 Seconds = CountdownTime - (Minutes * 60); // 카운트다운 시간을 초 단위로 변환한다.
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TfHud->Alert->WarmupTime->SetText(FText::FromString(CountDownText));
	}
}
void ATFPlayerController::SetHUDGrenadeCount(int32 Grenades)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->GrenadeAmount;
	if (bHUDVaild) {
		FString GrenadeText = FString::Printf(TEXT("%d"), Grenades); // 현재 탄약과 최대 탄약을 포맷팅한다.
		TfHud->CharacterOverlay->GrenadeAmount->SetText(FText::FromString(GrenadeText));
	}
	else {
		bInitializeGrenades = true;
		HUDGrenades = Grenades; // HUDGrenades를 초기화한다.
	}
}
void ATFPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// HUD 포인터 갱신 및 폴링 재개
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
	StartUIKeepAlive();
}

void ATFPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;

	if (HasAuthority())
	{
		TFGameMode = TFGameMode == nullptr ? Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)) : TFGameMode;
		if (TFGameMode)
		{
			TimeLeft = TFGameMode->GetCountdownTime(); // ★ 서버는 권위값 사용
		}
	}
	else
	{
		// 클라는 로컬 계산(혹은 서버 푸시를 기다림)
		if (MatchState == MatchState::WaitingToStart)
			TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
		else if (MatchState == MatchState::InProgress)
			TimeLeft = MatchTime - GetServerTime() + LevelStartingTime;
		else if (MatchState == MatchState::CoolDown)
			TimeLeft = CoolDownTime - GetServerTime() + LevelStartingTime;
	}

	// 매 틱 갱신(조건 비교 없이)
	if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::CoolDown)
		SetHUDAlertCountDown(TimeLeft);
	else if (MatchState == MatchState::InProgress)
		SetHUDMatchCountdown(TimeLeft);

	if (MatchState == MatchState::InProgress)
	{
		HideAlertIfAny();
	}
}
void ATFPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime; // 시간 동기화가 얼마나 진행되었는지 저장한다.
	if (IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency) // 로컬 컨트롤러인 경우 시간 동기화 주기가 지났다면
	{
		ServerRequestimeSync(GetWorld()->GetTimeSeconds()); // 서버에 현재 시간을 요청한다.
		TimeSyncRunningTime = 0.f; // 시간 동기화 진행 시간을 초기화한다.
	}
}

void ATFPlayerController::ClientPlayHitConfirmSound_Implementation(USoundCue* HitSound)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySound2D(this, HitSound);
	}
}
void ATFPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest; // 왕복 시간을 계산한다.
	float ServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f); // 서버 시간을 계산한다.
	ClientServerDelta = ServerTime - GetWorld()->GetTimeSeconds(); // 클라이언트와 서버 간의 시간 차이를 계산한다.
}
void ATFPlayerController::ServerRequestimeSync_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds(); // 서버의 현재 시간을 가져온다.
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt); // 클라이언트에게 서버 시간을 보고한다.
}

void ATFPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer(); // 부모 클래스의 ReceivedPlayer 호출
	if (IsLocalController()) { // 로컬 컨트롤러인 경우
		ServerRequestimeSync(GetWorld()->GetTimeSeconds()); // 로컬 컨트롤러인 경우 서버에 현재 시간을 요청한다.
	}
}

float ATFPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds(); // 서버 권한이 있는 경우 현재 월드의 시간을 반환한다.
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta; // 서버 권한이 없는 경우 클라이언트와 서버 간의 시간 차이를 더하여 시간을 반환한다.
}



void ATFPlayerController::OnRep_MatchState() {
	
	OnMatchStateSet(MatchState);
}
void ATFPlayerController::OnMatchStateSet(FName State) {
	MatchState = State;          // 상태만 반영
	ApplyMatchStateUI_Local(MatchState);  // 로컬 UI 전환
}
void ATFPlayerController::HandleMatchHasStarted() {
	if (HasAuthority()) Client_HideAlert(); else HideAlertIfAny(); 
	EnsureOverlayAndSync();
}
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
			TfHud->CharacterOverlay = nullptr; // ★ 쿨다운에서만 제거
		}
		// 쿨다운에선 Alert 필요(승자 정보 등)
		EnsureAlert();
	}
}
void ATFPlayerController::HandleCoolDown()
{
TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
if (!TfHud) return;

// 킬피드 타이머 정리(뒤늦은 콜백으로 파괴된 위젯 접근 방지)
GetWorldTimerManager().ClearTimer(KillFeedClearTimer);

// 오버레이는 쿨다운 동안 숨김 (겹침 방지)
if (TfHud->CharacterOverlay)
{
	TfHud->CharacterOverlay->RemoveFromParent();
	TfHud->CharacterOverlay = nullptr;
}

EnsureAlert(); // Alert 위젯 보장

if (TfHud->Alert)
{
	TfHud->Alert->SetVisibility(ESlateVisibility::Visible);
	TfHud->Alert->AlertText->SetText(FText::FromString(TEXT("New Match Starting In:")));

	if (ATFGameState* GS = Cast<ATFGameState>(UGameplayStatics::GetGameState(this)))
	{
		// 1) 원래 의도: GameState가 관리하는 TopScorePlayers 사용
		TArray<ATFPlayerState*> TopPlayers = GS->TopScorePlayers;

		// 2) 폴백: 혹시 TopScorePlayers가 비었으면 직접 PlayerArray 스캔해서 동점자 계산
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

		// 3) 나에게 보여줄 문구 구성
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

// 쿨다운 동안 조작 비활성화(오발/상태 꼬임 방지)
if (ATimeFractureCharacter* MyChar = Cast<ATimeFractureCharacter>(GetPawn()))
{
	if (MyChar->GetCombatComponent())
	{
		MyChar->bDisableGameplay = true;
		MyChar->GetCombatComponent()->FireButtonPressed(false);
	}
}
}
void ATFPlayerController::ClientJoinMatch_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown)
{
	MatchState = StateOfMatch;
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	CoolDownTime = CoolDown;
	ApplyMatchStateUI_Local(MatchState);
}
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
		// 인게임 시작: Alert 완전 제거 + Overlay 보장 + Combat에서 HUD 한방 동기화
		HideAlertIfAny();
		EnsureOverlayAndSync();

		// (서버·레이스 방지) 0.1초 후 한 번 더 Alert 싹 정리
		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, [this]()
			{
				HideAlertIfAny();
			}, 0.1f, false);
	}
	else if (State == MatchState::CoolDown)
	{
		HandleCoolDown(); // 내부에서 오버레이 제거 + Alert 보장 + 승자표시 수행
	}
}
void ATFPlayerController::ServerCheckMatchState_Implementation()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	ATFGameMode* GameMode = Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)); // 게임 모드를 가져온다.
	if (GameMode) {
		CoolDownTime = GameMode->CoolDownTime;// 게임 모드의 쿨다운 시간을 가져온다.
		WarmupTime = GameMode->WarmupTime; // 게임 모드의 웜업 시간을 가져온다.
		MatchTime = GameMode->MatchTime; // 게임 모드의 매치 시간을 가져온다.
		LevelStartingTime = GameMode->LevelStartingTime; // 게임 모드의 레벨 시작 시간을 가져온다.
		MatchState = GameMode->GetMatchState(); // 게임 모드의 매치 상태를 가져온다.
		ClientJoinMatch(MatchState, WarmupTime, MatchTime, LevelStartingTime,CoolDownTime); // 클라이언트에게 매치 상태를 전송한다.
	}
}
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
		ServerCheckMatchState();  // 재시작 후 MatchState 복구
	}

	// 이제 위젯 다 떠 있으면 타이머 종료
	if (CharacterOverlay)
	{
		GetWorldTimerManager().ClearTimer(PollInitTimerHandle);
	}
}
void ATFPlayerController::ServerSendChatMessage_Implementation(const FString& Message)//0913 채팅
{
	APlayerState* PS = GetPlayerState<APlayerState>();
	FString SenderName = PS ? PS->GetPlayerName() : TEXT("Unknown");

	// 모든 플레이어에게 메시지 브로드캐스트
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATFPlayerController* PC = Cast<ATFPlayerController>(*It);
		if (PC)
		{
			PC->ClientReceiveChatMessage(SenderName, Message);
		}
	}
}
void ATFPlayerController::ClientReceiveChatMessage_Implementation(const FString& Sender, const FString& Message)//0913 채팅
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
void ATFPlayerController::RequestStartMatch()
{
	if (HasAuthority())  // 호스트만 가능
	{
		ALBGameMode* LobbyGM = Cast<ALBGameMode>(UGameplayStatics::GetGameMode(this));
		if (LobbyGM)
		{
			LobbyGM->ServerStartMatch();
		}
	}
}
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
				// Row 위젯 생성 (WBP_ScoreboardRow)
				UUserWidget* Row = CreateWidget<UUserWidget>(this, ScoreboardRowClass);
				if (!Row) continue;

				// 내부 TextBlock 찾기
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

				// ScrollBox에 추가
				PlayerListBox->AddChild(Row);
			}
		}
	}
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // 부모 클래스의 BeginPlay 호출
	ServerCheckMatchState();  // 클라 정보 요청

	if (IsLocalController() && LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
			LobbyWidget->HideStartButton();
		}
	}

	// InputMode 설정
	if (HasAuthority() && GetWorld()->GetAuthGameMode()->GetClass()->GetName().Contains("TFGameMode"))
	{
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}

	// HUD 초기화 보장
	GetWorldTimerManager().SetTimer(PollInitTimerHandle, this, &ATFPlayerController::PollInit, 0.2f, true);
	StartUIKeepAlive();
}
void ATFPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // 부모 클래스의 Tick 호출
	SetHUDTime(); // HUD의 시간을 설정한다.
	CheckTimeSync(DeltaTime); // 시간 동기화를 확인한다.
	if (!CharacterOverlay && MatchState == MatchState::InProgress)
	{
		PollInit();
		EnsureOverlayAndSync();
	}

}
void ATFPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // 부모 클래스의 GetLifetimeReplicatedProps 호출
	DOREPLIFETIME(ATFPlayerController, MatchState); // MatchState 변수를 복제한다.
}
void ATFPlayerController::StartUIKeepAlive()
{
	GetWorldTimerManager().SetTimer(UIKeepAliveTimerHandle, this, &ATFPlayerController::UIKeepAliveTick, 1.5f, true);
}
void ATFPlayerController::UIKeepAliveTick()
{
	if (MatchState == MatchState::InProgress)
	{
		EnsureOverlayAndSync();  // 없으면 생성, 있으면 Combat에서 HUD값 강제 푸시
		HideAlertIfAny();        // 진행 중에는 Alert 잔존 방지
	}
}
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
void ATFPlayerController::EnsureAlert()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (TfHud && !TfHud->Alert)
	{
		TfHud->AddAlert();
	}
}
void ATFPlayerController::HideAlertIfAny()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (!TfHud) return;

	bool bRemoved = false;

	// 1) 우리가 추적하던 포인터 제거
	if (TfHud->Alert)
	{
		TfHud->Alert->SetVisibility(ESlateVisibility::Hidden);
		TfHud->Alert->RemoveFromParent();
		TfHud->Alert = nullptr;
		bRemoved = true;
	}

		if (TfHud->AlertClass)
		{
			TArray<UUserWidget*> Existing;
			UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Existing, TfHud->AlertClass, /*TopLevelOnly=*/false);
			for (UUserWidget* W : Existing)
			{
				if (W) { W->RemoveFromParent(); }
			}
		}
}
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

void ATFPlayerController::Client_HideAlert_Implementation()
{
	HideAlertIfAny();
}