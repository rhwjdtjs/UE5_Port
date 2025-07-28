// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerController.h"
#include "UnrealProject_7A/HUD/TFHUD.h" // TFHUD ��������� ���Խ�Ų��.
#include "UnrealProject_7A/HUD/CharacterOverlay.h" 
#include "Components/ProgressBar.h" // UProgressBar ��� ���� ����
#include "Components/TextBlock.h" // UTextBlock ��� ���� ����
#include "UnrealProject_7A/Character/TimeFractureCharacter.h" // TimeFractureCharacter ��������� ���Խ�Ų��.
#include "UnrealProject_7A/Weapon/Weapon.h" // Weapon ��������� ���Խ�Ų��.
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
}
void ATFPlayerController::SetHUDScore(float Score)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud�� nullptr�̸� GetHUD()�� ���� HUD�� ��������, �׷��� ������ ������ TfHud�� ����Ѵ�.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->ScoreAmount;
	if(bHUDVaild) {
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score)); // ���� �ؽ�Ʈ�� �������Ѵ�.
		TfHud->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
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
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f); // ī��Ʈ�ٿ� �ð��� �� ������ ��ȯ�Ѵ�.
		int32 Seconds = CountdownTime - (Minutes * 60); // ī��Ʈ�ٿ� �ð��� �� ������ ��ȯ�Ѵ�.
		FString CountDownText=FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		TfHud->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
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
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime()); // ���� �ð��� �������� ���� �ð��� �� ������ ����Ѵ�.
	if(CountdownInt != SecondsLeft) // ���� �ð��� ������ �ٸ���
	{
		SetHUDMatchCountdown((MatchTime - GetServerTime())); // HUD�� ��ġ ī��Ʈ�ٿ��� �����Ѵ�.
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
void ATFPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // �θ� Ŭ������ Tick ȣ��
	SetHUDTime(); // HUD�� �ð��� �����Ѵ�.
	CheckTimeSync(DeltaTime); // �ð� ����ȭ�� Ȯ���Ѵ�.

}
// CharacterHUD ��������� ���Խ�Ų��.
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // �θ� Ŭ������ BeginPlay ȣ��

	TfHud = Cast<ATFHUD>(GetHUD()); // HUD�� �����´�.
}
