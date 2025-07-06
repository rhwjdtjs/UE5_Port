// Fill out your copyright notice in the Description page of Project Settings.


#include "TFHUD.h"

void ATFHUD::DrawHUD()
{
	Super::DrawHUD(); //drawhud�� ���̽� �Լ��� �ҷ���
	FVector2D ViewportSize; //ȭ�� ũ�⸦ ������ ����
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize); //ȭ�� ũ�⸦ ������
		const FVector2D ViewportCenter(ViewportSize.X / ViewportDiv, ViewportSize.Y / ViewportDiv); //ȭ�� �߾� ��ǥ ���

		if (HUDPackage.CrosshairsCenter) {
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsLeft) {
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsRight) {
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsTop) {
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsBottom) {
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter); //�߾� ũ�ν���� �׸���
		}
	}
}

void ATFHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter)
{
	const float TextureWidth = Texture->GetSizeX(); //ũ�ν���� �ؽ����� �ʺ�
	const float TextureHeight = Texture->GetSizeY(); //ũ�ν���� �ؽ����� ����
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / ViewportDiv), ViewportCenter.Y - (TextureHeight / ViewportDiv)); //ũ�ν���� �ؽ��ĸ� �׸� ��ġ ���
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, FLinearColor::White); //ũ�ν���� �ؽ��ĸ� ȭ�鿡 �׸���
}
