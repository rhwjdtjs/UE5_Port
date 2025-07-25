// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerController.h"
#include "UnrealProject_7A/HUD/TFHUD.h" // TFHUD 헤더파일을 포함시킨다.
#include "UnrealProject_7A/HUD/CharacterOverlay.h" 
#include "Components/ProgressBar.h" // UProgressBar 헤더 파일 포함
#include "Components/TextBlock.h" // UTextBlock 헤더 파일 포함
#include "UnrealProject_7A/Character/TimeFractureCharacter.h" // TimeFractureCharacter 헤더파일을 포함시킨다.
#include "UnrealProject_7A/Weapon/Weapon.h" // Weapon 헤더파일을 포함시킨다.
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
}
void ATFPlayerController::SetHUDScore(float Score)
{
	TfHud = TfHud == nullptr ? Cast<ATFHUD>(GetHUD()) : TfHud; // TfHud가 nullptr이면 GetHUD()를 통해 HUD를 가져오고, 그렇지 않으면 기존의 TfHud를 사용한다.
	bool bHUDVaild = TfHud && TfHud->CharacterOverlay && TfHud->CharacterOverlay->ScoreAmount;
	if(bHUDVaild) {
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score)); // 점수 텍스트를 포맷팅한다.
		TfHud->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
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
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f); // 카운트다운 시간을 분 단위로 변환한다.
		int32 Seconds = CountdownTime - (Minutes * 60); // 카운트다운 시간을 초 단위로 변환한다.
		FString CountDownText=FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds);
		TfHud->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
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
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetWorld()->GetTimeSeconds()); // 현재 시간에서 매치 시간을 빼서 남은 시간을 계산한다.
	if(CountdownInt != SecondsLeft) // 남은 시간이 이전과 다르면
	{
		SetHUDMatchCountdown((MatchTime - GetWorld()->GetTimeSeconds())); // HUD의 매치 카운트다운을 설정한다.
	}
	CountdownInt = SecondsLeft; // 남은 시간을 정수형으로 저장한다.
}
void ATFPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime); // 부모 클래스의 Tick 호출
	SetHUDTime(); // HUD의 시간을 설정한다.
}
// CharacterHUD 헤더파일을 포함시킨다.
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // 부모 클래스의 BeginPlay 호출

	TfHud = Cast<ATFHUD>(GetHUD()); // HUD를 가져온다.
}
