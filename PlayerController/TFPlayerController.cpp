// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerController.h"
#include "UnrealProject_7A/HUD/TFHUD.h" // TFHUD 헤더파일을 포함시킨다.
#include "UnrealProject_7A/HUD/CharacterOverlay.h" 
#include "Components/ProgressBar.h" // UProgressBar 헤더 파일 포함
#include "Components/TextBlock.h" // UTextBlock 헤더 파일 포함
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
// CharacterHUD 헤더파일을 포함시킨다.
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // 부모 클래스의 BeginPlay 호출

	TfHud=Cast<ATFHUD>(GetHUD()); // HUD를 가져온다.
}
