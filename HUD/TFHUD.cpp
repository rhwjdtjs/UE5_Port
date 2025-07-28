// Fill out your copyright notice in the Description page of Project Settings.


#include "TFHUD.h"
#include "GameFrameWork/PlayerController.h"
#include "CharacterOverlay.h"
#include "Alert.h"
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
	APlayerController* PlayerController = GetOwningPlayerController(); //현재 플레이어 컨트롤러를 가져옴
	if (PlayerController && AlertClass) //플레이어 컨트롤러가 유효하고 캐릭터 오버레이 클래스가 설정되어 있다면
	{
		Alert = CreateWidget<UAlert>(PlayerController, AlertClass); //캐릭터 오버레이 위젯을 생성
		Alert->AddToViewport(); //위젯을 화면에 추가
	}
}

void ATFHUD::BeginPlay()
{
	Super::BeginPlay(); //베이스 클래스의 BeginPlay 함수를 호출
}

void ATFHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController=GetOwningPlayerController(); //현재 플레이어 컨트롤러를 가져옴
	if (PlayerController && CharacterOverlayClass) //플레이어 컨트롤러가 유효하고 캐릭터 오버레이 클래스가 설정되어 있다면
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass); //캐릭터 오버레이 위젯을 생성
		CharacterOverlay->AddToViewport(); //위젯을 화면에 추가
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
