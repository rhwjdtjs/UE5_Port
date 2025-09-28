// Fill out your copyright notice in the Description page of Project Settings.


#include "TFHUD.h"
#include "GameFrameWork/PlayerController.h"
#include "CharacterOverlay.h"
#include "Alert.h"
#include "ChatWidget.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
void ATFHUD::DrawHUD()
{
	Super::DrawHUD(); //drawhud의 베이스 함수를 불러옴
	FVector2D ViewportSize; //화면 크기를 저장할 변수
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize); //화면 크기를 가져옴
		const FVector2D ViewportCenter(ViewportSize.X / ViewportDiv, ViewportSize.Y / ViewportDiv); //화면 중앙 좌표 계산
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread; //크로스헤어 스프레드 최대값과 현재 스프레드를 곱하여 스프레드 크기를 계산
		if (HUDPackage.CrosshairsCenter) {
			FVector2D Spread(0.f, 0.f); //크로스헤어 스프레드 초기화
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter,Spread ,HUDPackage.CrosshairColor); //중앙 크로스헤어 그리기
		}
		if (HUDPackage.CrosshairsLeft) {
			FVector2D Spread(-SpreadScaled, 0.f); //왼쪽 크로스헤어 스프레드 계산
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor); //중앙 크로스헤어 그리기
		}
		if (HUDPackage.CrosshairsRight) {
			FVector2D Spread(SpreadScaled, 0.f); //오른쪽 크로스헤어 스프레드 계산
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairColor); //중앙 크로스헤어 그리기
		}
		if (HUDPackage.CrosshairsTop) {
			FVector2D Spread(0.f, -SpreadScaled); //위쪽 크로스헤어 스프레드 계산
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairColor); //중앙 크로스헤어 그리기
		}
		if (HUDPackage.CrosshairsBottom) {
			FVector2D Spread(0.f, SpreadScaled); //아래쪽 크로스헤어 스프레드 계산
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter,Spread, HUDPackage.CrosshairColor); //중앙 크로스헤어 그리기
		}
	}
}

void ATFHUD::AddAlert()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !AlertClass) return;

	// 0) 뷰포트에 같은 클래스의 Alert가 있으면 전부 제거(중복 AddToViewport 방지)
	TArray<UUserWidget*> Existing;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC, Existing, AlertClass, false);
	for (UUserWidget* W : Existing)
	{
		if (W) W->RemoveFromParent();
	}

	// 1) 우리가 추적하던 포인터도 정리
	if (Alert)
	{
		Alert->RemoveFromParent();
		Alert = nullptr;
	}

	// 2) 새로 생성 + 상단 레이어에 추가
	Alert = CreateWidget<UAlert>(PC, AlertClass);
	if (Alert)
	{
		Alert->AddToViewport(5);
	}
}

void ATFHUD::AddChatWidget()
{
	APlayerController* PC = GetOwningPlayerController();
	if (PC && ChatWidgetClass)
	{
		ChatWidget = CreateWidget<UChatWidget>(PC, ChatWidgetClass);
		if (ChatWidget)
		{
			ChatWidget->AddToViewport();
		}
	}
}

void ATFHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ATFHUD::AddCharacterOverlay()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !CharacterOverlayClass) return;

	// 0) 혹시 뷰포트에 같은 클래스의 위젯이 남아 있으면 전부 제거(블프/코드 중복 추가 케이스 방지)
	TArray<UUserWidget*> Existing;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(PC, Existing, CharacterOverlayClass, false);
	for (UUserWidget* W : Existing)
	{
		if (W) W->RemoveFromParent();
	}

	// 1) 우리가 추적하던 포인터도 정리
	if (CharacterOverlay)
	{
		CharacterOverlay->RemoveFromParent();
		CharacterOverlay = nullptr;
	}

	// 2) 새로 생성 + 뷰포트에 추가
	CharacterOverlay = CreateWidget<UCharacterOverlay>(PC, CharacterOverlayClass);
	if (CharacterOverlay)
	{
		CharacterOverlay->AddToViewport(1);

		// 3) 생성 직후 Combat → HUD 값 즉시 동기화 (리스폰/복구 직후 빈 화면 방지)
		if (APawn* P = PC->GetPawn())
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
}

void ATFHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX(); //크로스헤어 텍스쳐의 너비
	const float TextureHeight = Texture->GetSizeY(); //크로스헤어 텍스쳐의 높이
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / ViewportDiv) + Spread.X,
		ViewportCenter.Y - (TextureHeight / ViewportDiv)+ Spread.Y); //크로스헤어 텍스쳐를 그릴 위치 계산
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, CrosshairColor); //크로스헤어 텍스쳐를 화면에 그리기
}
