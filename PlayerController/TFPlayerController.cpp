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
// ���: �������� ų�ǵ� �޽����� Ŭ���̾�Ʈ���� ����.
// ����: Server �� Client RPC�� ȣ��Ǿ� Ŭ���̾�Ʈ���� HUD ���� ����.
// ============================================================
void ATFPlayerController::ClientAddKillFeedMessage_Implementation(const FString& Killer, const FString& Victim)
{
	AddKillFeedMessage(Killer, Victim); // ���� �޽��� �߰��� ���� �Լ����� ó��
}

// ============================================================
// ���: HUD�� ų�ǵ� UI�� ������ ������ �׿������� ǥ��.
// �˰���:
//   1. ��ġ�� ���� ��(InProgress)�� ���� ǥ��.
//   2. KillFeedBox�� �����ϴ��� Ȯ�� �� ���ο� TextBlock �߰�.
//   3. 5�� �� �ڵ����� Ŭ�����ϴ� Ÿ�̸� ����.
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

	// KillFeed �ڵ� Ŭ���� Ÿ�̸�
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
// ���: Ű �Է� ���� �Լ�.
// ����: "Quit"�� �޴� ǥ��, "Scoreboard"�� ������ ǥ��/����.
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
// ���: ������ ǥ��.
// �˰���:
//   1. Scoreboard ������ ������ ���� �� Viewport�� �߰�.
//   2. �̹� �ִ��� ������ ������ �ٽ� Ȱ��ȭ.
//   3. ǥ�� �� �׻� UpdateScoreboard() ȣ��� �ֽ� ������ ����.
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
// ���: ������ ����� (Scoreboard Ű�� ���� �� ȣ���)
// ============================================================
void ATFPlayerController::HideScoreboard()
{
	if (ScoreboardWidget)
	{
		ScoreboardWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

// ============================================================
// ���: ESC/Quit Ű �Է� �� ���� �޴��� ǥ���ϰų� ����.
// �˰���:
//   1. ������ ������ ����.
//   2. ���� �����ִ� ���¿� ���� ����(MenuSetup) �Ǵ� �ݱ�(MenuTearDown) ����.
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
// ���: Ŭ���̾�Ʈ�� ������� �� Killed ������ ǥ��.
// �˰���:
//   1. ���� ���� �� Viewport�� �߰�.
//   2. 3�� �� �ڵ� ����.
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
// ���: �÷��̾ �ٸ� �÷��̾ óġ���� �� Kill ������ ǥ��.
// �˰���:
//   1. ���� ���� �� ȭ�鿡 ǥ��.
//   2. 3�� �� �ڵ� ���� Ÿ�̸� ����.
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
// ���: HUD ü��(Health) ǥ�� ������Ʈ
// �˰���:
//   1. TFHUD�� CharacterOverlay�� ���� ���� Ȯ��.
//   2. ��ȿ�ϴٸ� ProgressBar�� TextBlock�� ���� ü�� �ݿ�.
//   3. HUD�� ���� �������� �ʾҴٸ� ���� �ӽ� ���� �� ���߿� �ʱ�ȭ ó��.
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
// ���: HUD ����(Shield) ǥ�� ������Ʈ
// �˰���:
//   1. CharacterOverlay�� Shield ���� ������ �����ϸ� �ٷ� ����.
//   2. �������� ������ HUD �ʱ�ȭ ������ ����ǵ��� �� ����.
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
// ���: HUD ����(Score) ǥ�� ����
// �˰���:
//   1. CharacterOverlay�� ScoreAmount ���� Ȯ��.
//   2. HUD�� ���� ��� �ʱ�ȭ ����.
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
// ���: HUD�� �й�(Defeats) �� ǥ�� ����
// �˰���:
//   1. CharacterOverlay�� DefeatsAmount�� �����ϴ��� Ȯ��.
//   2. ������ �ٷ� ǥ��, ������ �ʱ�ȭ�� ������ ����.
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
// ���: ���� ���� ź��(Weapon Ammo) HUD ǥ�� ����
// �˰���:
//   1. HUD�� AmmoAmount �ؽ�Ʈ�� ����.
//   2. HUD�� ���� �غ���� �ʾҴٸ� �� ����.
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
// ���: ���� ź��(Carried Ammo) HUD ǥ�� ����
// �˰���:
//   1. HUD�� CarriedAmmoAmount �ؽ�Ʈ�� ����.
//   2. HUD �̻��� �� �ӽ� ���� �� PollInit���� ����.
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
// ���: ��ġ �ð� ī��Ʈ�ٿ� ǥ��
// �˰���:
//   1. ��ġ �ð��� 0���� ������ ǥ�ø� ����.
//   2. �� ���� �ð��� ��:�� ���·� ������ �� �ؽ�Ʈ ����.
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
// ���: �˸�(Alert) ������ ī��Ʈ�ٿ� �ؽ�Ʈ ����
// �˰���:
//   1. TfHud.Alert ������ WarmupTime �ؽ�Ʈ ���� ���� Ȯ��.
//   2. ��ȿ�ϸ� ��:�� �������� ǥ��.
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
// ���: HUD ����ź ���� ǥ�� ����
// �˰���:
//   1. HUD�� �����ϸ� GrenadeAmount �ؽ�Ʈ�� ǥ��.
//   2. HUD�� ���� ������ ���߿� �ʱ�ȭ �� �ݿ�.
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
// ���: �÷��̾ Pawn�� ������ �� HUD �� �Է� ����.
// �˰���:
//   1. HUD ĳ�� ���� �� PollInit Ÿ�̸� Ȱ��ȭ.
//   2. ���� ���� Pawn(ĳ����)�� ü��, ���� HUD ��� �ݿ�.
//   3. ���콺 Ŀ�� ��Ȱ��ȭ �� ���� ���� �Է� ��� ����.
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

	StartUIKeepAlive(); // UI ���� Ÿ�̸� ����
}

// ============================================================
// ���: HUD�� �ð� �ؽ�Ʈ�� ����.
// �˰���:
//   1. ������ ��� GameMode���� ���� ī��Ʈ�ٿ� ���� ����.
//   2. Ŭ���� ��� ���� �ð� �������� ���ÿ��� ���.
//   3. ���º�(���, ����, ��ٿ�)�� ���� �˸��� HUD ������ ǥ��.
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
// ���: ������ Ŭ���̾�Ʈ �� �ð� ���� ���� �˰���.
// �˰���:
//   1. �� �����Ӹ��� ���� �ð� ����.
//   2. ���� �ֱ�(TimeSyncFrequency)���� ������ ���� �ð��� ��û.
//   3. ���� ������ �޾� ClientServerDelta ������ ���.
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
// ���: �����κ��� Ŭ���̾�Ʈ���� �ð� ������ �������.
// �˰���:
//   1. Ŭ�� ��û�� ���� �ð�(TimeOfClientRequest)��
//      ������ ���� �ð�(TimeServerReceivedClientRequest)�� �̿��� �պ� �ð� ���.
//   2. Ŭ��-���� �ð���(ClientServerDelta) ������ ����.
// ============================================================
void ATFPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float ServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);
	ClientServerDelta = ServerTime - GetWorld()->GetTimeSeconds();
}

// ============================================================
// ���: Ŭ���̾�Ʈ �� ���� �ð� ��û.
// �˰���:
//   1. Ŭ���̾�Ʈ�� �ڽ��� ��û �ð��� ����.
//   2. ������ ��û�� ���� ��� ���� ���� �ð��� ������ ���� RPC ����.
// ============================================================
void ATFPlayerController::ServerRequestimeSync_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

// ============================================================
// ���: �÷��̾ ��Ʈ�ѷ��� ���� �� ���� (��Ʈ��ũ �ʱ�ȭ ���� ȣ���).
// ����: ���� ��Ʈ�ѷ��� ��� ��� ������ �ð� ����ȭ ��û�� ����.
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
// ���: ���� �ð� ��� (Ŭ���̾�Ʈ ���� ����).
// �˰���:
//   - ������ �ڽ��� World Time�� �״�� ���.
//   - Ŭ��� ClientServerDelta(������)�� ���� ���� �ð��� ������Ŵ.
// ============================================================
float ATFPlayerController::GetServerTime()
{
	if (HasAuthority())
		return GetWorld()->GetTimeSeconds();
	else
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

// ============================================================
// ���: MatchState�� ������ �� ȣ��Ǵ� �Լ�.
// ����: OnRep_ �Լ��� Ŭ���̾�Ʈ���� ���� ���� �ٲ���� �� Ʈ���ŵ�.
// ============================================================
void ATFPlayerController::OnRep_MatchState()
{
	OnMatchStateSet(MatchState);
}

// ============================================================
// ���: ��ġ ���� ���� �� UI �� ���� ó�� ����.
// �˰���:
//   1. MatchState �� ����.
//   2. ApplyMatchStateUI_Local()�� HUD/Alert ���� ����.
// ============================================================
void ATFPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	ApplyMatchStateUI_Local(MatchState);
}
// ============================================================
// ���: ��ġ�� ���۵� �� ȣ��.
// �˰���:
//   1. ������ ��� Ŭ���̾�Ʈ���� Alert ���� ��� ����(Client_HideAlert).
//   2. Ŭ��� ���� Alert ���� ����.
//   3. ��������(ü��/ź�� ǥ�� UI)�� ������ ������ ���� �� HUD ����ȭ.
// ============================================================
void ATFPlayerController::HandleMatchHasStarted() {
	if (HasAuthority())
		Client_HideAlert();
	else
		HideAlertIfAny();

	EnsureOverlayAndSync();
}

// ============================================================
// ���: �÷��̾ Pawn�� �Ҿ��� ��(�װų� ���� ���� ��) ȣ��.
// �˰���:
//   1. HUD ������ ��Ȯ�� �� Ÿ�̸� ����.
//   2. ��ٿ� ���¿����� �������� ���� �� Alert ���� ǥ��.
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
// ���: ��ٿ� ����(���� ���� �� ���� ǥ�� �� ���).
// �˰���:
//   1. ų�ǵ� ���� �� HUD �������� ����.
//   2. Alert ���� ���� �� ��Winner�� �ؽ�Ʈ ����.
//   3. GameState�� TopScorePlayers �������� ���� �Ǵ�.
//   4. Ŭ�� ���� ��Ȱ��ȭ�� ���� ����.
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

	EnsureAlert(); // ��ٿ� �˸� ����

	if (TfHud->Alert)
	{
		TfHud->Alert->SetVisibility(ESlateVisibility::Visible);
		TfHud->Alert->AlertText->SetText(FText::FromString(TEXT("New Match Starting In:")));

		if (ATFGameState* GS = Cast<ATFGameState>(UGameplayStatics::GetGameState(this)))
		{
			TArray<ATFPlayerState*> TopPlayers = GS->TopScorePlayers;

			// ���� ����: TopScorePlayers�� ����� ��� ���� �ְ� ���� Ž��
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

			// ǥ�� ���� ����
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

	// ���� ��Ȱ��ȭ (���� ���� ����)
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
// ���: Ŭ���̾�Ʈ�� �����κ��� ��ġ ������ ������ �� ����.
// �˰���:
//   1. ��ġ ����, �ð�, ����, ��ٿ�, ���� ���۽ð� ���� ����.
//   2. ApplyMatchStateUI_Local() ȣ���Ͽ� UI�� �ش� ���¿� �°� ����.
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
// ���: ���� UI�� ���� ��ġ ���¿� �°� ��ȯ.
// �˰���:
//   1. ���(WaitingToStart): Alert ���� �� ǥ�� (��Match Starting In����).
//   2. ����(InProgress): Alert ���� �� Overlay ���� �� HUD ����ȭ.
//   3. ��ٿ�(CoolDown): HandleCoolDown ȣ���Ͽ� ���� ǥ�� �� ���.
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
// ���: ������ ���� ��ġ ���¸� Ŭ���̾�Ʈ���� ����ȭ.
// �˰���:
//   1. GameMode���� �ð� ���� ������(Warmup, Match, Cooldown ��) ȹ��.
//   2. Ŭ���̾�Ʈ RPC(ClientJoinMatch)�� �ش� �� ����.
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
// ���: HUD�� ���� ���� �ʱ�ȭ�� ����(Polling) ������� ����.
// �˰���:
//   1. HUD �� CharacterOverlay�� ������ ������ �ֱ������� �˻�.
//   2. bInitializeXXX ���� true�� �׸���� ���� HUD�� �ݿ�.
//   3. ��ġ ���°� None�̸� ������ ���� ����ȭ ��û.
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
// ���: �������� ä�� �޽��� ��ε�ĳ��Ʈ.
// �˰���:
//   1. �޽����� ������ Ŭ���̾�Ʈ�� PlayerState���� �̸� ����.
//   2. ���� ���� ���� ��� PlayerController ��ȸ.
//   3. �� Ŭ���̾�Ʈ�� ClientReceiveChatMessage RPC ȣ��.
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
// ���: Ŭ���̾�Ʈ���� ä�� �޽����� �����ϰ� HUD�� ǥ��.
// �˰���:
//   1. HUD �� ChatWidget ���� ���� Ȯ��.
//   2. ChatWidget�� ������ ��� ����(AddChatWidget).
//   3. AddChatMessage�� �̸� + ���� �߰� �� ǥ�� ����.
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
// ���: �κ� ȣ��Ʈ���� ������ ��ư�� ǥ�� ��ȣ�� ����.
// �˰���:
//   1. LobbyWidget�� ���� �������� ���� ��� �ణ ���� �� ��õ�.
//   2. LobbyWidget�� ��ȿ�ϸ� ��ư ǥ�� �� �Է¸�� ��ȯ.
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
// ���: �������� �Ⱦ�(������ ����) ȿ���� Ŭ���̾�Ʈ ���� ���.
// �˰���:
//   1. ���尡 �����ϸ� �ش� ��ġ���� ���.
//   2. Niagara �ý���(��ƼŬ ����Ʈ)�� �����Ǿ� ������ ����.
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
// ���: Ŭ���̾�Ʈ�� ȣ��Ʈ�� ��� ���� ���� ��û.
// �˰���:
//   1. ���� GameMode�� LBGameMode�� ĳ����.
//   2. ���� ������ ���� ���� ServerStartMatch() ����.
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
// ���: ������(Scoreboard) UI ����.
// �˰���:
//   1. Scoreboard ������ ScrollBox(PlayerListBox) Ž��.
//   2. ���� GameState�� ��� PlayerState�� ��ȸ.
//   3. �̸� / ų�� / ������ �ؽ�Ʈ�� Row ������ ä������.
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
// ���: �÷��̾� ��Ʈ�ѷ��� BeginPlay (���� �ʱ�ȭ).
// �˰���:
//   1. �������� ��ġ ���� ���� ��û(ServerCheckMatchState).
//   2. �κ� ������ ��� LobbyWidget ���� �� ��Ȱ��ȭ ��ư ����.
//   3. �ΰ��ӿ����� ���콺Ŀ�� �� �Է� ��� ��ȯ.
//   4. HUD�� �غ�� ������ PollInit Ÿ�̸ӷ� �ݺ� �˻�.
//   5. UI ������ �ֱ� Ÿ�̸�(UIKeepAlive) ����.
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
// ���: �� �����Ӹ��� �ð���HUD ����ȭ ����.
// �˰���:
//   1. HUD ī��Ʈ�ٿ� �ؽ�Ʈ ����(SetHUDTime).
//   2. Ŭ��-���� �ð� ����ȭ(CheckTimeSync).
//   3. Overlay�� ������� ��� �����.
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
// ���: ��Ʈ��ũ ���� ���� ���.
// ����: MatchState�� Ŭ���̾�Ʈ�� �����ǵ��� ����.
// ============================================================
void ATFPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATFPlayerController, MatchState);
}

// ============================================================
// ���: UI �ڵ� ���� �ֱ� Ÿ�̸� ����.
// �˰���:
//   - ���� �ֱ�(1.5��)���� UIKeepAliveTick ����.
// ============================================================
void ATFPlayerController::StartUIKeepAlive()
{
	GetWorldTimerManager().SetTimer(UIKeepAliveTimerHandle, this, &ATFPlayerController::UIKeepAliveTick, 1.5f, true);
}

// ============================================================
// ���: UI ���� �˰���.
// �˰���:
//   1. ��ġ�� ���� ���� ���� ����.
//   2. Overlay�� ������ ���� �� HUD ������ ���� Ǫ��.
//   3. Alert ������ ���� ������ ���� ���� (�þ� ��ø ����).
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
// ���: Overlay ���� ���� ���� + HUD ���� ����ȭ.
// �˰���:
//   1. HUD ĳ�� ���� �� CharacterOverlay Ȯ��.
//   2. Overlay�� ������ AddCharacterOverlay()�� ����.
//   3. Pawn�� CombatComponent���� PushAllHUDFromCombat() ȣ��.
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
// ���: Alert ���� ���� ����.
// �˰���:
//   1. HUD ĳ�� ����.
//   2. Alert ������ ������ AddAlert()���� ��� ����.
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
// ���: Alert ���� ���� ����.
// �˰���:
//   1. ���� HUD �����Ϳ��� Alert ���� �� NULL ó��.
//   2. HUD Ŭ���� ������� ���� ������ ���� RemoveFromParent.
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
// ���: Ŭ���̾�Ʈ���� Alert ������ ��������� ǥ��.
// �˰���:
//   1. Alert ������ ������ AddAlert() ȣ��.
//   2. ǥ�� �� ���� �ؽ�Ʈ�� ���ڷκ��� ����.
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
// ���: Ŭ���̾�Ʈ���� Alert �����.
// �˰���:
//   - HideAlertIfAny() ȣ���Ͽ� Alert ���� ���� ��ü ����.
// ============================================================
void ATFPlayerController::Client_HideAlert_Implementation()
{
	HideAlertIfAny();
}


