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
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth; // HUDHealth와 HUDMaxHealth를 초기화한다.
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
		bInitializeCharacterOverlay = true;
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
		bInitializeCharacterOverlay = true;
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
}
void ATFPlayerController::SetHUDCarriedAmmo(int32 Ammos)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDVaild) {
		FString CarriedAmmo = FString::Printf(TEXT("%d"), Ammos); // 현재 탄약과 최대 탄약을 포맷팅한다.
		TfHud->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmo));
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
void ATFPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); // 부모 클래스의 OnPossess 호출
	ATimeFractureCharacter* TfCharacter = Cast<ATimeFractureCharacter>(InPawn); // InPawn을 TimeFractureCharacter로 캐스팅한다.
	if(TfCharacter) {
		SetHUDHealth(TfCharacter->GetHealth(), TfCharacter->GetMaxHealth()); // TfCharacter의 체력과 최대 체력을 HUD에 설정한다.
	}
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
	if (IsLocalController() && TfHud) {
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
				TfHud->Alert->InfoText->SetText(FText()); // InfoText를 비운다.
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
				SetHUDHealth(HUDHealth, HUDMaxHealth); // HUDHealth와 HUDMaxHealth를 설정한다.
				SetHUDScore(HUDScore); // HUDScore를 설정한다.
				SetHUDDefeats(HUDDefeats); // HUDDefeats를 설정한다.
			}
		}
	}
}
// CharacterHUD 헤더파일을 포함시킨다.
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // 부모 클래스의 BeginPlay 호출
	ServerCheckMatchState(); // 서버에 매치 상태를 확인 요청
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