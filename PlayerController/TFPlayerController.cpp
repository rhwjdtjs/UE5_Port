// Fill out your copyright notice in the Description page of Project Settings.


#include "TFPlayerController.h"
#include "UnrealProject_7A/HUD/TFHUD.h" // TFHUD ��������� ���Խ�Ų��.
#include "UnrealProject_7A/HUD/CharacterOverlay.h" 
#include "Components/ProgressBar.h" // UProgressBar ��� ���� ����
#include "Components/TextBlock.h" // UTextBlock ��� ���� ����
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
// CharacterHUD ��������� ���Խ�Ų��.
void ATFPlayerController::BeginPlay()
{
	Super::BeginPlay(); // �θ� Ŭ������ BeginPlay ȣ��

	TfHud=Cast<ATFHUD>(GetHUD()); // HUD�� �����´�.
}
