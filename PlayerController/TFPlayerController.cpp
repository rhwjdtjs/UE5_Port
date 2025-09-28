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
#include "UnrealProject_7A/GameState/TFGameState.h" // TFPlayerState ��������� ���Խ�Ų��.
#include "UnrealProject_7A/PlayerState/TFPlayerState.h" // TFPlayerState ��������� ���Խ�Ų��.
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
#include "Blueprint/WidgetBlueprintLibrary.h" // Alert/Overlay ������ �����ϰ� ���� ������ �� ���


void ATFPlayerController::ClientAddKillFeedMessage_Implementation(const FString& Killer, const FString& Victim)
{
	AddKillFeedMessage(Killer, Victim); // Ŭ�󿡼� ���� �޽��� �߰� ����
}
void ATFPlayerController::AddKillFeedMessage(const FString& Killer, const FString& Victim)
{
	// InProgress ���¿����� ǥ��
	if (MatchState != MatchState::InProgress) return;

	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
	if (!TfHud) return;

	// Overlay�� ������ �������� �ʰ� ���� (��ħ ����)
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
	Super::SetupInputComponent(); // �θ� Ŭ������ SetupInputComponent ȣ��
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &ATFPlayerController::ShowReturnToMainMenu); // "Quit" �׼��� ������ �� ShowReturnToMainMenu �Լ� ȣ��
	InputComponent->BindAction("Scoreboard", IE_Pressed, this, &ATFPlayerController::ShowScoreboard);
	InputComponent->BindAction("Scoreboard", IE_Released, this, &ATFPlayerController::HideScoreboard);
}
void ATFPlayerController::ShowScoreboard()
{
	if (!ScoreboardClass) return;

	// �����/Ʈ���� ��: �����ʹ� ����־ ������ �׾��ų� ����Ʈ���� �������� �� ����
	if (!IsValid(ScoreboardWidget))
	{
		ScoreboardWidget = CreateWidget<UUserWidget>(this, ScoreboardClass);
		if (ScoreboardWidget) { ScoreboardWidget->AddToViewport(50); }
	}
	else if (!ScoreboardWidget->IsInViewport())
	{
		// ����Ʈ�� ������ �ٽ� ���
		ScoreboardWidget->AddToViewport(50);
	}

	if (ScoreboardWidget)
	{
		ScoreboardWidget->SetVisibility(ESlateVisibility::Visible);
		UpdateScoreboard(); // �� �� �׻� ����
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
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuClass); // ReturnToMainMenu ���� ����
	}
	if (ReturnToMainMenu) {
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen) {
			ReturnToMainMenu->MenuSetup(); // �޴� ����
		}
		else {
			ReturnToMainMenu->MenuTearDown(); // �޴� ����
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
		// �ʿ��ϸ� Ÿ�̸ӷ� �ڵ� ����
		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, [this]()
			{
				if (KilledWidgetInstance) { KilledWidgetInstance->RemoveFromParent(); KilledWidgetInstance = nullptr; }
			}, 3.0f, false); // 3�� �� ����
	}
}
void ATFPlayerController::ClientShowKillWidget_Implementation()
{
	if (KillWidgetClass == nullptr) return;
	KillWidgetInstance = CreateWidget<UUserWidget>(this, KillWidgetClass);
	if (KillWidgetInstance)
	{
		KillWidgetInstance->AddToViewport();
		// �ʿ��ϸ� Ÿ�̸ӷ� �ڵ� ����
		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, [this]()
			{
				if (KillWidgetInstance) { KillWidgetInstance->RemoveFromParent(); KillWidgetInstance = nullptr; }
			}, 3.0f, false); // 3�� �� ����
	}
}
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
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth; // HUDHealth�� HUDMaxHealth�� �ʱ�ȭ�Ѵ�.
	}
}
void ATFPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->ShieldBar && TfHud->CharacterOverlay->ShieldText;
	if (bHUDVaild) {
		const float ShieldPercent = Shield / MaxShield; // ü�� ������ ����Ѵ�.
		TfHud->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent); // ü�¹��� ������ �����Ѵ�.
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield)); // ü�� �ؽ�Ʈ�� �������Ѵ�.
		TfHud->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText)); // ü�� �ؽ�Ʈ�� �����Ѵ�.
	}
	else {
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield; // HUDHealth�� HUDMaxHealth�� �ʱ�ȭ�Ѵ�.
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
		bInitializeScore = true;
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
		bInitializeDefeats = true;
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
		else {
			bInitializeAmmos = true;
			HUDAmmos = Ammos; // HUDAmmos�� �ʱ�ȭ�Ѵ�.
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
	else {
		bInitializeCarriedAmmos = true;
		HUDCarriedAmmos = Ammos; // HUDCarriedAmmos�� �ʱ�ȭ�Ѵ�.
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
void ATFPlayerController::SetHUDGrenadeCount(int32 Grenades)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->GrenadeAmount;
	if (bHUDVaild) {
		FString GrenadeText = FString::Printf(TEXT("%d"), Grenades); // ���� ź��� �ִ� ź���� �������Ѵ�.
		TfHud->CharacterOverlay->GrenadeAmount->SetText(FText::FromString(GrenadeText));
	}
	else {
		bInitializeGrenades = true;
		HUDGrenades = Grenades; // HUDGrenades�� �ʱ�ȭ�Ѵ�.
	}
}
void ATFPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// HUD ������ ���� �� ���� �簳
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
			TimeLeft = TFGameMode->GetCountdownTime(); // �� ������ ������ ���
		}
	}
	else
	{
		// Ŭ��� ���� ���(Ȥ�� ���� Ǫ�ø� ��ٸ�)
		if (MatchState == MatchState::WaitingToStart)
			TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
		else if (MatchState == MatchState::InProgress)
			TimeLeft = MatchTime - GetServerTime() + LevelStartingTime;
		else if (MatchState == MatchState::CoolDown)
			TimeLeft = CoolDownTime - GetServerTime() + LevelStartingTime;
	}

	// �� ƽ ����(���� �� ����)
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
	TimeSyncRunningTime += DeltaTime; // �ð� ����ȭ�� �󸶳� ����Ǿ����� �����Ѵ�.
	if (IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency) // ���� ��Ʈ�ѷ��� ��� �ð� ����ȭ �ֱⰡ �����ٸ�
	{
		ServerRequestimeSync(GetWorld()->GetTimeSeconds()); // ������ ���� �ð��� ��û�Ѵ�.
		TimeSyncRunningTime = 0.f; // �ð� ����ȭ ���� �ð��� �ʱ�ȭ�Ѵ�.
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



void ATFPlayerController::OnRep_MatchState() {
	
	OnMatchStateSet(MatchState);
}
void ATFPlayerController::OnMatchStateSet(FName State) {
	MatchState = State;          // ���¸� �ݿ�
	ApplyMatchStateUI_Local(MatchState);  // ���� UI ��ȯ
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
			TfHud->CharacterOverlay = nullptr; // �� ��ٿ���� ����
		}
		// ��ٿ�� Alert �ʿ�(���� ���� ��)
		EnsureAlert();
	}
}
void ATFPlayerController::HandleCoolDown()
{
TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
if (!TfHud) return;

// ų�ǵ� Ÿ�̸� ����(�ڴ��� �ݹ����� �ı��� ���� ���� ����)
GetWorldTimerManager().ClearTimer(KillFeedClearTimer);

// �������̴� ��ٿ� ���� ���� (��ħ ����)
if (TfHud->CharacterOverlay)
{
	TfHud->CharacterOverlay->RemoveFromParent();
	TfHud->CharacterOverlay = nullptr;
}

EnsureAlert(); // Alert ���� ����

if (TfHud->Alert)
{
	TfHud->Alert->SetVisibility(ESlateVisibility::Visible);
	TfHud->Alert->AlertText->SetText(FText::FromString(TEXT("New Match Starting In:")));

	if (ATFGameState* GS = Cast<ATFGameState>(UGameplayStatics::GetGameState(this)))
	{
		// 1) ���� �ǵ�: GameState�� �����ϴ� TopScorePlayers ���
		TArray<ATFPlayerState*> TopPlayers = GS->TopScorePlayers;

		// 2) ����: Ȥ�� TopScorePlayers�� ������� ���� PlayerArray ��ĵ�ؼ� ������ ���
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

		// 3) ������ ������ ���� ����
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

// ��ٿ� ���� ���� ��Ȱ��ȭ(����/���� ���� ����)
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
		// �ΰ��� ����: Alert ���� ���� + Overlay ���� + Combat���� HUD �ѹ� ����ȭ
		HideAlertIfAny();
		EnsureOverlayAndSync();

		// (���������̽� ����) 0.1�� �� �� �� �� Alert �� ����
		FTimerHandle Th;
		GetWorldTimerManager().SetTimer(Th, [this]()
			{
				HideAlertIfAny();
			}, 0.1f, false);
	}
	else if (State == MatchState::CoolDown)
	{
		HandleCoolDown(); // ���ο��� �������� ���� + Alert ���� + ����ǥ�� ����
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
		ServerCheckMatchState();  // ����� �� MatchState ����
	}

	// ���� ���� �� �� ������ Ÿ�̸� ����
	if (CharacterOverlay)
	{
		GetWorldTimerManager().ClearTimer(PollInitTimerHandle);
	}
}
void ATFPlayerController::ServerSendChatMessage_Implementation(const FString& Message)//0913 ä��
{
	APlayerState* PS = GetPlayerState<APlayerState>();
	FString SenderName = PS ? PS->GetPlayerName() : TEXT("Unknown");

	// ��� �÷��̾�� �޽��� ��ε�ĳ��Ʈ
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATFPlayerController* PC = Cast<ATFPlayerController>(*It);
		if (PC)
		{
			PC->ClientReceiveChatMessage(SenderName, Message);
		}
	}
}
void ATFPlayerController::ClientReceiveChatMessage_Implementation(const FString& Sender, const FString& Message)//0913 ä��
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
	if (HasAuthority())  // ȣ��Ʈ�� ����
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
				// Row ���� ���� (WBP_ScoreboardRow)
				UUserWidget* Row = CreateWidget<UUserWidget>(this, ScoreboardRowClass);
				if (!Row) continue;

				// ���� TextBlock ã��
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

				// ScrollBox�� �߰�
				PlayerListBox->AddChild(Row);
			}
		}
	}
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // �θ� Ŭ������ BeginPlay ȣ��
	ServerCheckMatchState();  // Ŭ�� ���� ��û

	if (IsLocalController() && LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
			LobbyWidget->HideStartButton();
		}
	}

	// InputMode ����
	if (HasAuthority() && GetWorld()->GetAuthGameMode()->GetClass()->GetName().Contains("TFGameMode"))
	{
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
	}

	// HUD �ʱ�ȭ ����
	GetWorldTimerManager().SetTimer(PollInitTimerHandle, this, &ATFPlayerController::PollInit, 0.2f, true);
	StartUIKeepAlive();
}
void ATFPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // �θ� Ŭ������ Tick ȣ��
	SetHUDTime(); // HUD�� �ð��� �����Ѵ�.
	CheckTimeSync(DeltaTime); // �ð� ����ȭ�� Ȯ���Ѵ�.
	if (!CharacterOverlay && MatchState == MatchState::InProgress)
	{
		PollInit();
		EnsureOverlayAndSync();
	}

}
void ATFPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // �θ� Ŭ������ GetLifetimeReplicatedProps ȣ��
	DOREPLIFETIME(ATFPlayerController, MatchState); // MatchState ������ �����Ѵ�.
}
void ATFPlayerController::StartUIKeepAlive()
{
	GetWorldTimerManager().SetTimer(UIKeepAliveTimerHandle, this, &ATFPlayerController::UIKeepAliveTick, 1.5f, true);
}
void ATFPlayerController::UIKeepAliveTick()
{
	if (MatchState == MatchState::InProgress)
	{
		EnsureOverlayAndSync();  // ������ ����, ������ Combat���� HUD�� ���� Ǫ��
		HideAlertIfAny();        // ���� �߿��� Alert ���� ����
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

	// 1) �츮�� �����ϴ� ������ ����
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