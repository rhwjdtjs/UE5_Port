// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerController.h"
#include "UnrealProject_7A/HUD/TFHUD.h" // TFHUD ��������� ���Խ�Ų��.
#include "UnrealProject_7A/HUD/CharacterOverlay.h" 
#include "Components/ProgressBar.h" // UProgressBar ��� ���� ����
#include "Components/TextBlock.h" // UTextBlock ��� ���� ����
#include "UnrealProject_7A/Character/TimeFractureCharacter.h" // TimeFractureCharacter ��������� ���Խ�Ų��.
#include "UnrealProject_7A/Weapon/Weapon.h" // Weapon ��������� ���Խ�Ų��.
#include "Net/UnrealNetwork.h" // ��Ʈ��ũ ���� ��� ���� ����
#include "UnrealProject_7A/GameMode/TFGameMode.h" // TFGameMode ��������� ���Խ�Ų��.
#include "UnrealProject_7A/HUD/Alert.h" // Alert ��������� ���Խ�Ų��.
#include "Kismet/GameplayStatics.h" // �����÷��� ����ƽ�� ��� ���� ����
#include "UnrealProject_7A/TFComponents/CBComponent.h" // CBComponent ��������� ���Խ�Ų��.
void ATFPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->HealthBar && TfHud->CharacterOverlay->HealthText;
	if (bHUDVaild) {
		const float HealthPercent = Health / MaxHealth; // ü�� ������ ����Ѵ�.
		TfHud->CharacterOverlay->HealthBar->SetPercent(HealthPercent); // ü�¹��� ������ �����Ѵ�.
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth)); // ü�� �ؽ�Ʈ�� �������Ѵ�.
		TfHud->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText)); // ü�� �ؽ�Ʈ�� �����Ѵ�.
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth; // HUDHealth�� HUDMaxHealth�� �ʱ�ȭ�Ѵ�.
	}
}
void ATFPlayerController::SetHUDScore(float Score)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->ScoreAmount;
	if(bHUDVaild) {
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score)); // ���� �ؽ�Ʈ�� �������Ѵ�.
		TfHud->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDScore = Score; // HUDScore�� �ʱ�ȭ�Ѵ�.
	}
}
void ATFPlayerController::SetHUDDefeats(int32 Defeats)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->DefeatsAmount;
	if (bHUDVaild) {
		FString DefeatText = FString::Printf(TEXT("%d"), Defeats); // ���� �ؽ�Ʈ�� �������Ѵ�.
		TfHud->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatText));
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats; // HUDDefeats�� �ʱ�ȭ�Ѵ�.
	}
}
void ATFPlayerController::SetHUDWeaponAmmo(int32 Ammos)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
		bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->AmmoAmount;
		if (bHUDVaild) {
			FString AmmoText = FString::Printf(TEXT("%d"), Ammos); // ���� ź��� �ִ� ź���� �������Ѵ�.
			TfHud->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}
void ATFPlayerController::SetHUDCarriedAmmo(int32 Ammos)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDVaild) {
		FString CarriedAmmo = FString::Printf(TEXT("%d"), Ammos); // ���� ź��� �ִ� ź���� �������Ѵ�.
		TfHud->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmo));
	}
}
void ATFPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->MatchCountDownText;
	if (bHUDVaild) {
		if (CountdownTime < 0.f) {
			TfHud->CharacterOverlay->MatchCountDownText->SetText(FText());
			return; // ī��Ʈ�ٿ� �ð��� 0���� ������ �ؽ�Ʈ�� ����.
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f); // ī��Ʈ�ٿ� �ð��� �� ������ ��ȯ�Ѵ�.
		int32 Seconds = CountdownTime - (Minutes * 60); // ī��Ʈ�ٿ� �ð��� �� ������ ��ȯ�Ѵ�.
		FString CountDownText=FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		TfHud->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}
void ATFPlayerController::SetHUDAlertCountDown(float CountdownTime)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	bool bHUDVaild = TfHud && TfHud->Alert && TfHud->Alert->WarmupTime;
	if (bHUDVaild) {
		if (CountdownTime < 0.f) {
			TfHud->Alert->WarmupTime->SetText(FText());
			return; // ī��Ʈ�ٿ� �ð��� 0���� ������ �ؽ�Ʈ�� ����.
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f); // ī��Ʈ�ٿ� �ð��� �� ������ ��ȯ�Ѵ�.
		int32 Seconds = CountdownTime - (Minutes * 60); // ī��Ʈ�ٿ� �ð��� �� ������ ��ȯ�Ѵ�.
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TfHud->Alert->WarmupTime->SetText(FText::FromString(CountDownText));
	}
}
void ATFPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); // �θ� Ŭ������ OnPossess ȣ��
	ATimeFractureCharacter* TfCharacter = Cast<ATimeFractureCharacter>(InPawn); // InPawn�� TimeFractureCharacter�� ĳ�����Ѵ�.
	if(TfCharacter) {
		SetHUDHealth(TfCharacter->GetHealth(), TfCharacter->GetMaxHealth()); // TfCharacter�� ü�°� �ִ� ü���� HUD�� �����Ѵ�.
	}
}

void ATFPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f; // ���� �ð��� �ʱ�ȭ�Ѵ�.
	if (MatchState == MatchState::WaitingToStart) {
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}// ���� ������ �� ���� �ð��� ����Ѵ�.
	else if (MatchState == MatchState::InProgress) {
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::CoolDown) {
		TimeLeft = CoolDownTime + WarmupTime+MatchTime -GetServerTime() + LevelStartingTime;
	}
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft); // ���� �ð��� �������� ���� �ð��� �� ������ ����Ѵ�.
	//if (HasAuthority()) // ���� ������ �ִ� ���
//	{
	//	TFGameMode = TFGameMode == nullptr ? Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)) : TFGameMode; // TFGameMode�� nullptr�̸� ���� ���� ��带 ��������, �׷��� ������ ������ TFGameMode�� ����Ѵ�.
	//	if (TFGameMode) {
	//		SecondsLeft = FMath::CeilToInt(TFGameMode->GetCountdownTime() + LevelStartingTime); // TFGameMode���� ī��Ʈ�ٿ� �ð��� �����´�.
	//	}
	//}
		if (CountdownInt != SecondsLeft) // ���� �ð��� ������ �ٸ���
		{
			if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::CoolDown) {
				SetHUDAlertCountDown(TimeLeft); // ���� ������ �� HUD�� ī��Ʈ�ٿ��� �����Ѵ�.
			}
			if (MatchState == MatchState::InProgress) {
				SetHUDMatchCountdown(TimeLeft); // ���� ���� ���� �� HUD�� ī��Ʈ�ٿ��� �����Ѵ�.
			}
		}
	
	CountdownInt = SecondsLeft; // ���� �ð��� ���������� �����Ѵ�.
}
void ATFPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime; // �ð� ����ȭ�� �󸶳� ����Ǿ����� �����Ѵ�.
	if (IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency) // ���� ��Ʈ�ѷ��� ��� �ð� ����ȭ �ֱⰡ �����ٸ�
	{
		ServerRequestimeSync(GetWorld()->GetTimeSeconds()); // ������ ���� �ð��� ��û�Ѵ�.
		TimeSyncRunningTime = 0.f; // �ð� ����ȭ ���� �ð��� �ʱ�ȭ�Ѵ�.
	}
}
void ATFPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest; // �պ� �ð��� ����Ѵ�.
	float ServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f); // ���� �ð��� ����Ѵ�.
	ClientServerDelta = ServerTime - GetWorld()->GetTimeSeconds(); // Ŭ���̾�Ʈ�� ���� ���� �ð� ���̸� ����Ѵ�.
}
void ATFPlayerController::ServerRequestimeSync_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds(); // ������ ���� �ð��� �����´�.
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt); // Ŭ���̾�Ʈ���� ���� �ð��� �����Ѵ�.
}

void ATFPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer(); // �θ� Ŭ������ ReceivedPlayer ȣ��
	if (IsLocalController()) { // ���� ��Ʈ�ѷ��� ���
		ServerRequestimeSync(GetWorld()->GetTimeSeconds()); // ���� ��Ʈ�ѷ��� ��� ������ ���� �ð��� ��û�Ѵ�.
	}
}
float ATFPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds(); // ���� ������ �ִ� ��� ���� ������ �ð��� ��ȯ�Ѵ�.
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta; // ���� ������ ���� ��� Ŭ���̾�Ʈ�� ���� ���� �ð� ���̸� ���Ͽ� �ð��� ��ȯ�Ѵ�.
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
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
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
				TfHud->Alert->SetVisibility(ESlateVisibility::Visible); // Alert ������ ���̰� �Ѵ�.
				FString AlertText(" New Match Starting In: ");
				TfHud->Alert->AlertText->SetText(FText::FromString(AlertText)); // Alert ������ �ؽ�Ʈ�� �����Ѵ�.
				TfHud->Alert->InfoText->SetText(FText()); // InfoText�� ����.
			}
		}
	}
	ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(GetPawn());
	if (TFCharacter && TFCharacter->GetCombatComponent()) {
		TFCharacter->bDisableGameplay = true; // ĳ������ �����÷��̸� ��Ȱ��ȭ�Ѵ�.
		TFCharacter->GetCombatComponent()->FireButtonPressed(false); // ���� ������Ʈ�� �߻� ��ư�� �����ٰ� �����Ѵ�.
	}
}
void ATFPlayerController::ClientJoinMatch_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float CoolDown)
{
	MatchState = StateOfMatch;// ��ġ ���¸� �����Ѵ�.
	WarmupTime = Warmup;// ���� �ð��� �����Ѵ�.
	MatchTime = Match;// ��ġ �ð��� �����Ѵ�.
	LevelStartingTime = StartingTime; // ���� ���� �ð��� �����Ѵ�.
	CoolDownTime = CoolDown; // ��ٿ� �ð��� �����Ѵ�.
	OnMatchStateSet(MatchState); // ��ġ ���°� ����Ǿ��� �� ȣ��Ǵ� �Լ��� �����Ѵ�.
	if (TfHud && MatchState == MatchState::WaitingToStart) {
		TfHud->AddAlert(); // HUD�� �˸� ������ �߰��Ѵ�.
	}
}
void ATFPlayerController::ServerCheckMatchState_Implementation()
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	ATFGameMode* GameMode = Cast<ATFGameMode>(UGameplayStatics::GetGameMode(this)); // ���� ��带 �����´�.
	if (GameMode) {
		CoolDownTime = GameMode->CoolDownTime;// ���� ����� ��ٿ� �ð��� �����´�.
		WarmupTime = GameMode->WarmupTime; // ���� ����� ���� �ð��� �����´�.
		MatchTime = GameMode->MatchTime; // ���� ����� ��ġ �ð��� �����´�.
		LevelStartingTime = GameMode->LevelStartingTime; // ���� ����� ���� ���� �ð��� �����´�.
		MatchState = GameMode->GetMatchState(); // ���� ����� ��ġ ���¸� �����´�.
		ClientJoinMatch(MatchState, WarmupTime, MatchTime, LevelStartingTime,CoolDownTime); // Ŭ���̾�Ʈ���� ��ġ ���¸� �����Ѵ�.
	}
}
void ATFPlayerController::PollInit() {
	if (CharacterOverlay == nullptr) {
		if (TfHud && TfHud->CharacterOverlay) {
			CharacterOverlay = TfHud->CharacterOverlay; // TfHud�� CharacterOverlay�� �����´�.
			if (CharacterOverlay) {
				SetHUDHealth(HUDHealth, HUDMaxHealth); // HUDHealth�� HUDMaxHealth�� �����Ѵ�.
				SetHUDScore(HUDScore); // HUDScore�� �����Ѵ�.
				SetHUDDefeats(HUDDefeats); // HUDDefeats�� �����Ѵ�.
			}
		}
	}
}
// CharacterHUD ��������� ���Խ�Ų��.
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // �θ� Ŭ������ BeginPlay ȣ��
	ServerCheckMatchState(); // ������ ��ġ ���¸� Ȯ�� ��û
}
void ATFPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // �θ� Ŭ������ Tick ȣ��
	SetHUDTime(); // HUD�� �ð��� �����Ѵ�.
	CheckTimeSync(DeltaTime); // �ð� ����ȭ�� Ȯ���Ѵ�.
	PollInit(); // ���� ���� �Լ� �ʱ�ȭ
}
void ATFPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // �θ� Ŭ������ GetLifetimeReplicatedProps ȣ��
	DOREPLIFETIME(ATFPlayerController, MatchState); // MatchState ������ �����Ѵ�.
}