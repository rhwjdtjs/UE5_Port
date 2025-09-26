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
void ATFPlayerController::ClientAddKillFeedMessage_Implementation(const FString& Killer, const FString& Victim)
{
	AddKillFeedMessage(Killer, Victim); // Ŭ�󿡼� ���� �޽��� �߰� ����
}
void ATFPlayerController::AddKillFeedMessage(const FString& Killer, const FString& Victim)
{

		TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud;
		if (!TfHud || !TfHud->CharacterOverlay) return;
		FTimerHandle KillFeedClearTimer;
		// BP: CharacterOverlay �ȿ� ScrollBox �̸��� ��Ȯ�� "KillFeedBox" ��!
		UScrollBox* KillFeedBox = Cast<UScrollBox>(
			TfHud->CharacterOverlay->GetWidgetFromName(TEXT("KillFeedBox")));
		if (!KillFeedBox) return;

		
		UWidgetTree* WT = TfHud->CharacterOverlay->WidgetTree;
		if (!WT) return;

		UTextBlock* NewMessage = WT->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		const FString Msg = FString::Printf(TEXT("%s killed %s"), *Killer, *Victim);
		NewMessage->SetText(FText::FromString(Msg));
		KillFeedBox->AddChild(NewMessage);
		// 3�� �� ��ü Ŭ����
		GetWorld()->GetTimerManager().ClearTimer(KillFeedClearTimer);
		GetWorld()->GetTimerManager().SetTimer(
			KillFeedClearTimer,
			[KillFeedBox]()
			{
				KillFeedBox->ClearChildren();
			},
			3.f, false
		);
		// ������ �޽��� ���� (��: 5�� ����)
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

	//if (KilledWidgetInstance)
	//{
	//	KilledWidgetInstance->RemoveFromParent();
//		KilledWidgetInstance = nullptr;
//	}

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

	//if (KillWidgetInstance)
	//{
//		KillWidgetInstance->RemoveFromParent();
//		KillWidgetInstance = nullptr;
//	}

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
	Super::OnPossess(InPawn); // �θ� Ŭ������ OnPossess ȣ��
	ATimeFractureCharacter* TfCharacter = Cast<ATimeFractureCharacter>(InPawn); // InPawn�� TimeFractureCharacter�� ĳ�����Ѵ�.
	if(TfCharacter) {
		SetHUDHealth(TfCharacter->GetHealth(), TfCharacter->GetMaxHealth()); // TfCharacter�� ü�°� �ִ� ü���� HUD�� �����Ѵ�.
		SetHUDShield(TfCharacter->GetShield(), TfCharacter->GetMaxShield()); // TfCharacter�� ����� �ִ� ���带 HUD�� �����Ѵ�.
	}
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
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
				TfHud->Alert->SetVisibility(ESlateVisibility::Visible); // Alert ������ ���̰� �Ѵ�.
				FString AlertText(" New Match Starting In: ");
				TfHud->Alert->AlertText->SetText(FText::FromString(AlertText)); // Alert ������ �ؽ�Ʈ�� �����Ѵ�.
				ATFGameState* TFGameState = Cast<ATFGameState>(UGameplayStatics::GetGameState(this)); // ���� ���¸� �����´�.
				ATFPlayerState* TFPlayerState = GetPlayerState<ATFPlayerState>(); // �÷��̾� ���¸� �����´�.
				if (TFGameState && TFPlayerState) {
					TArray<ATFPlayerState*> TopPlayers = TFGameState->TopScorePlayers;// ���� �÷��̾���� �����´�.
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
							InfoTextString.Append(FString::Printf(TEXT("%s\n"), *tiedPlayer->GetPlayerName())); // �� �÷��̾��� �̸��� �߰��Ѵ�.
						}
					}
					TfHud->Alert->InfoText->SetText(FText::FromString(InfoTextString)); // InfoText�� ����.
				}
				
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
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth); // HUDHealth�� HUDMaxHealth�� �����Ѵ�.
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield); // HUDShield�� HUDMaxShield�� �����Ѵ�.
				if (bInitializeScore) SetHUDScore(HUDScore); // HUDScore�� �����Ѵ�.
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats); // HUDDefeats�� �����Ѵ�.
				if (bInitializeAmmos) SetHUDWeaponAmmo(HUDAmmos); // HUDAmmos�� �����Ѵ�.
				if (bInitializeCarriedAmmos) SetHUDCarriedAmmo(HUDCarriedAmmos); // HUDCarriedAmmos�� �����Ѵ�.
				ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(GetPawn());
				if (TFCharacter && TFCharacter->GetCombatComponent()) {
					if (bInitializeGrenades) SetHUDGrenadeCount(TFCharacter->GetCombatComponent()->GetGrenades()); // TFCharacter�� ����ź ������ �����Ѵ�.
				}

			}
		}
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
// CharacterHUD ��������� ���Խ�Ų��.
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // �θ� Ŭ������ BeginPlay ȣ��
	ServerCheckMatchState();

	if (IsLocalController() && LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<ULobbyWidget>(this, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
			LobbyWidget->HideStartButton(); // ������ ���� (ȣ��Ʈ��)
		}
	}
	if (HasAuthority())
	{
		if (GetWorld()->GetAuthGameMode()->GetClass()->GetName().Contains("TFGameMode")) // �ΰ��� ��� �̸�
		{
			bShowMouseCursor = false;

			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
		}
	}
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