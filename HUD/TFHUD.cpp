// Fill out your copyright notice in the Description page of Project Settings.


#include "TFHUD.h"

void ATFHUD::DrawHUD()
{
	Super::DrawHUD(); //drawhud의 베이스 함수를 불러옴
	FVector2D ViewportSize; //화면 크기를 저장할 변수
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize); //화면 크기를 가져옴
		const FVector2D ViewportCenter(ViewportSize.X / ViewportDiv, ViewportSize.Y / ViewportDiv); //화면 중앙 좌표 계산

		if (HUDPackage.CrosshairsCenter) {
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter); //중앙 크로스헤어 그리기
		}
		if (HUDPackage.CrosshairsLeft) {
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter); //중앙 크로스헤어 그리기
		}
		if (HUDPackage.CrosshairsRight) {
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter); //중앙 크로스헤어 그리기
		}
		if (HUDPackage.CrosshairsTop) {
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter); //중앙 크로스헤어 그리기
		}
		if (HUDPackage.CrosshairsBottom) {
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter); //중앙 크로스헤어 그리기
		}
	}
}

void ATFHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter)
{
	const float TextureWidth = Texture->GetSizeX(); //크로스헤어 텍스쳐의 너비
	const float TextureHeight = Texture->GetSizeY(); //크로스헤어 텍스쳐의 높이
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / ViewportDiv), ViewportCenter.Y - (TextureHeight / ViewportDiv)); //크로스헤어 텍스쳐를 그릴 위치 계산
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, FLinearColor::White); //크로스헤어 텍스쳐를 화면에 그리기
}
