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
void ATFPlayerController::ClientAddKillFeedMessage_Implementation(const FString& Killer, const FString& Victim)
{
	AddKillFeedMessage(Killer, Victim); // 클라에서 실제 메시지 추가 실행
}
void ATFPlayerController::AddKillFeedMessage(const FString& Killer, const FString& Victim)
{

		TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
		if (!TfHud || !TfHud->CharacterOverlay) return;
		FTimerHandle KillFeedClearTimer;
		// BP: CharacterOverlay 안에 ScrollBox 이름을 정확히 "KillFeedBox" 로!
		UScrollBox* KillFeedBox = Cast<UScrollBox>(
			TfHud->CharacterOverlay->GetWidgetFromName(TEXT("KillFeedBox")));
		if (!KillFeedBox) return;

		
		UWidgetTree* WT = TfHud->CharacterOverlay->WidgetTree;
		if (!WT) return;

		UTextBlock* NewMessage = WT->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		const FString Msg = FString::Printf(TEXT("%s killed %s"), *Killer, *Victim);
		NewMessage->SetText(FText::FromString(Msg));
		KillFeedBox->AddChild(NewMessage);
		// 3초 뒤 전체 클리어
		GetWorld()->GetTimerManager().ClearTimer(KillFeedClearTimer);
		GetWorld()->GetTimerManager().SetTimer(
			KillFeedClearTimer,
			[KillFeedBox]()
			{
				KillFeedBox->ClearChildren();
			},
			3.f, false
		);
		// 오래된 메시지 정리 (예: 5개 유지)
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
	if (ScoreboardWidget == nullptr && ScoreboardClass)
	{
		ScoreboardWidget = CreateWidget<UUserWidget>(this, ScoreboardClass);
		if (ScoreboardWidget)
		{
			ScoreboardWidget->AddToViewport(50);
		}
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

	//if (KilledWidgetInstance)
	//{
	//	KilledWidgetInstance->RemoveFromParent();
//		KilledWidgetInstance = nullptr;
//	}

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

	//if (KillWidgetInstance)
	//{
//		KillWidgetInstance->RemoveFromParent();
//		KillWidgetInstance = nullptr;
//	}

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
	Super::OnPossess(InPawn); // 부모 클래스의 OnPossess 호출
	ATimeFractureCharacter* TfCharacter = Cast<ATimeFractureCharacter>(InPawn); // InPawn을 TimeFractureCharacter로 캐스팅한다.
	if(TfCharacter) {
		SetHUDHealth(TfCharacter->GetHealth(), TfCharacter->GetMaxHealth()); // TfCharacter의 체력과 최대 체력을 HUD에 설정한다.
		SetHUDShield(TfCharacter->GetShield(), TfCharacter->GetMaxShield()); // TfCharacter의 쉴드와 최대 쉴드를 HUD에 설정한다.
	}
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void ATFPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f; // 남은 시간을 초기화한다.
	if (MatchState == MatchState::WaitingToStart) {
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}// 웜업 상태일 때 남은 시간을 계산한다.
	else if (MatchState == MatchState::InProgress) {
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::CoolDown) {
		TimeLeft = CoolDownTime + WarmupTime+MatchTime -GetServerTime() + LevelStartingTime;
	}
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft); // 서버 시간을 기준으로 남은 시간을 초 단위로 계산한다.
	//if (HasAuthority()) // 서버 권한이 있는 경우
//	{
	//	TFGameMode = TFGameMode == nullptr ? Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)) : TFGameMode; // TFGameMode가 nullptr이면 현재 게임 모드를 가져오고, 그렇지 않으면 기존의 TFGameMode를 사용한다.
	//	if (TFGameMode) {
	//		SecondsLeft = FMath::CeilToInt(TFGameMode->GetCountdownTime() + LevelStartingTime); // TFGameMode에서 카운트다운 시간을 가져온다.
	//	}
	//}
		if (CountdownInt != SecondsLeft) // 남은 시간이 이전과 다르면
		{
			if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::CoolDown) {
				SetHUDAlertCountDown(TimeLeft); // 웜업 상태일 때 HUD에 카운트다운을 설정한다.
			}
			if (MatchState == MatchState::InProgress) {
				SetHUDMatchCountdown(TimeLeft); // 게임 진행 중일 때 HUD에 카운트다운을 설정한다.
			}
		}
	
	CountdownInt = SecondsLeft; // 남은 시간을 정수형으로 저장한다.
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

void ATFPlayerController::OnMatchStateSet(FName State) {
	MatchState = State;
	if (MatchState == MatchState::InProgress) {
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::CoolDown) {
		HandleCoolDown();
	}
}

void ATFPlayerController::OnRep_MatchState() {
	if (MatchState == MatchState::InProgress) {
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::CoolDown) {
		HandleCoolDown();
	}
}
void ATFPlayerController::HandleMatchHasStarted() {
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	if (TfHud) {
		if (TfHud->CharacterOverlay == nullptr)
			TfHud->AddCharacterOverlay();
		if (TfHud->Alert) {
			TfHud->Alert->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
void ATFPlayerController::HandleCoolDown()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (IsLocalController() && TfHud) {
		if (TfHud->CharacterOverlay) {
			TfHud->CharacterOverlay->RemoveFromParent();
			bool bHUDVaild = TfHud->Alert && TfHud->Alert->AlertText && TfHud->Alert->InfoText;
			if (bHUDVaild) {
				TfHud->Alert->SetVisibility(ESlateVisibility::Visible); // Alert 위젯을 보이게 한다.
				FString AlertText(" New Match Starting In: ");
				TfHud->Alert->AlertText->SetText(FText::FromString(AlertText)); // Alert 위젯의 텍스트를 설정한다.
				ATFGameState* TFGameState = Cast<ATFGameState>(UGameplayStatics::GetGameState(this)); // 게임 상태를 가져온다.
				ATFPlayerState* TFPlayerState = GetPlayerState<ATFPlayerState>(); // 플레이어 상태를 가져온다.
				if (TFGameState && TFPlayerState) {
					TArray<ATFPlayerState*> TopPlayers = TFGameState->TopScorePlayers;// 상위 플레이어들을 가져온다.
					FString InfoTextString;
					if (TopPlayers.Num() == 0) {
						InfoTextString = FString("There is no Winner");
					}
					else if (TopPlayers.Num() == 1 && TopPlayers[0]==TFPlayerState) {
						InfoTextString = FString("You are the Winner");
					}
					else if (TopPlayers.Num() == 1) {
						InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
					}
					else if (TopPlayers.Num() > 1) {
						InfoTextString = FString("Players tield for the win: \n");
						for (auto tiedPlayer : TopPlayers) {
							InfoTextString.Append(FString::Printf(TEXT("%s\n"), *tiedPlayer->GetPlayerName())); // 각 플레이어의 이름을 추가한다.
						}
					}
					TfHud->Alert->InfoText->SetText(FText::FromString(InfoTextString)); // InfoText를 비운다.
				}
				
			}
		}
	}
	ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(GetPawn());
	if (TFCharacter && TFCharacter->GetCombatComponent()) {
		TFCharacter->bDisableGameplay = true; // 캐릭터의 게임플레이를 비활성화한다.
		TFCharacter->GetCombatComponent()->FireButtonPressed(false); // 전투 컴포넌트의 발사 버튼을 눌렀다고 설정한다.
	}
}
void ATFPlayerController::ClientJoinMatch_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown)
{
	MatchState = StateOfMatch;// 매치 상태를 설정한다.
	WarmupTime = Warmup;// 웜업 시간을 설정한다.
	MatchTime = Match;// 매치 시간을 설정한다.
	LevelStartingTime = StartingTime; // 레벨 시작 시간을 설정한다.
	CoolDownTime = CoolDown; // 쿨다운 시간을 설정한다.
	OnMatchStateSet(MatchState); // 매치 상태가 변경되었을 때 호출되는 함수를 실행한다.
	if (TfHud && MatchState == MatchState::WaitingToStart) {
		TfHud->AddAlert(); // HUD에 알림 위젯을 추가한다.
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
	if (CharacterOverlay == nullptr) {
		if (TfHud && TfHud->CharacterOverlay) {
			CharacterOverlay = TfHud->CharacterOverlay; // TfHud의 CharacterOverlay를 가져온다.
			if (CharacterOverlay) {
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth); // HUDHealth와 HUDMaxHealth를 설정한다.
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield); // HUDShield와 HUDMaxShield를 설정한다.
				if (bInitializeScore) SetHUDScore(HUDScore); // HUDScore를 설정한다.
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats); // HUDDefeats를 설정한다.
				if (bInitializeAmmos) SetHUDWeaponAmmo(HUDAmmos); // HUDAmmos를 설정한다.
				if (bInitializeCarriedAmmos) SetHUDCarriedAmmo(HUDCarriedAmmos); // HUDCarriedAmmos를 설정한다.
				ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(GetPawn());
				if (TFCharacter && TFCharacter->GetCombatComponent()) {
					if (bInitializeGrenades) SetHUDGrenadeCount(TFCharacter->GetCombatComponent()->GetGrenades()); // TFCharacter의 수류탄 개수를 설정한다.
				}

			}
		}
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
void ATFPlayerController::ClientPlayBulletWhiz_Implementation(const FVector& Location)
{
	if(BulletFlyBySound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			BulletFlyBySound,
			Location,
			1.f,
			1.f,
			0.f,
			BulletFlyByAttenuation
		);
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
/*
void ATFPlayerController::UpdateScoreboard()
{
	if (!ScoreboardWidget) return;

	UScrollBox* PlayerListBox = Cast<UScrollBox>(ScoreboardWidget->GetWidgetFromName(TEXT("PlayerListBox")));
	if (!PlayerListBox) return;

	PlayerListBox->ClearChildren();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATFPlayerController* PC = Cast<ATFPlayerController>(*It);
		if (!PC || !PC->PlayerState) continue;

		ATFPlayerState* PS = Cast<ATFPlayerState>(PC->PlayerState);
		if (!PS) continue;
		if (!ScoreboardRowClass) continue;
		UUserWidget* Row = CreateWidget<UUserWidget>(this, ScoreboardRowClass);
		if (!Row) continue;

		UTextBlock* NameText = Cast<UTextBlock>(Row->GetWidgetFromName(TEXT("NameText")));
		UTextBlock* KillsText = Cast<UTextBlock>(Row->GetWidgetFromName(TEXT("KillsText")));
		UTextBlock* DeathsText = Cast<UTextBlock>(Row->GetWidgetFromName(TEXT("DeathsText")));

		if (NameText) {
			FString DisplayString = PS->GetPlayerName() + TEXT(" / ");
			NameText->SetText(FText::FromString(DisplayString));
		}
		if (KillsText) {
			FString DisplayString = FString::FromInt(FMath::FloorToInt(PS->GetScore())) + TEXT(" / ");
			KillsText->SetText(FText::FromString(DisplayString));
		}
		if (DeathsText)
		{
			DeathsText->SetText(FText::AsNumber(PS->GetDefeats()));
		}

		PlayerListBox->AddChild(Row);
	}
	}
	*/
// CharacterHUD 헤더파일을 포함시킨다.
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // 부모 클래스의 BeginPlay 호출
	ServerCheckMatchState();

	if (IsLocalController() && LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
			LobbyWidget->HideStartButton(); // 무조건 숨김 (호스트라도)
		}
	}
	if (HasAuthority())
	{
		if (GetWorld()->GetAuthGameMode()->GetClass()->GetName().Contains("TFGameMode")) // 인게임 모드 이름
		{
			bShowMouseCursor = false;

			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
		}
	}
}
void ATFPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // 부모 클래스의 Tick 호출
	SetHUDTime(); // HUD의 시간을 설정한다.
	CheckTimeSync(DeltaTime); // 시간 동기화를 확인한다.
	PollInit(); // 허드와 같은 함수 초기화
}
void ATFPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // 부모 클래스의 GetLifetimeReplicatedProps 호출
	DOREPLIFETIME(ATFPlayerController, MatchState); // MatchState 변수를 복제한다.
}