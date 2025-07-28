// Fill out your copyright notice in the Description page of Project Settings.


#include "TFHUD.h"
#include "GameFrameWork/PlayerController.h"
#include "CharacterOverlay.h"
#include "Alert.h"
void ATFHUD::DrawHUD()
{
	Super::DrawHUD(); //drawhud�� ���̽� �Լ��� �ҷ���
	FVector2D ViewportSize; //ȭ�� ũ�⸦ ������ ����
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize); //ȭ�� ũ�⸦ ������
		const FVector2D ViewportCenter(ViewportSize.X / ViewportDiv, ViewportSize.Y / ViewportDiv); //ȭ�� �߾� ��ǥ ���
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread; //ũ�ν���� �������� �ִ밪�� ���� �������带 ���Ͽ� �������� ũ�⸦ ���
		if (HUDPackage.CrosshairsCenter) {
			FVector2D Spread(0.f, 0.f); //ũ�ν���� �������� �ʱ�ȭ
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter,Spread ,HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsLeft) {
			FVector2D Spread(-SpreadScaled, 0.f); //���� ũ�ν���� �������� ���
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsRight) {
			FVector2D Spread(SpreadScaled, 0.f); //������ ũ�ν���� �������� ���
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsTop) {
			FVector2D Spread(0.f, -SpreadScaled); //���� ũ�ν���� �������� ���
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
		if (HUDPackage.CrosshairsBottom) {
			FVector2D Spread(0.f, SpreadScaled); //�Ʒ��� ũ�ν���� �������� ���
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter,Spread, HUDPackage.CrosshairColor); //�߾� ũ�ν���� �׸���
		}
	}
}

void ATFHUD::AddAlert()
{
	APlayerController* PlayerController = GetOwningPlayerController(); //���� �÷��̾� ��Ʈ�ѷ��� ������
	if (PlayerController && AlertClass) //�÷��̾� ��Ʈ�ѷ��� ��ȿ�ϰ� ĳ���� �������� Ŭ������ �����Ǿ� �ִٸ�
	{
		Alert = CreateWidget<UAlert>(PlayerController, AlertClass); //ĳ���� �������� ������ ����
		Alert->AddToViewport(); //������ ȭ�鿡 �߰�
	}
}

void ATFHUD::BeginPlay()
{
	Super::BeginPlay(); //���̽� Ŭ������ BeginPlay �Լ��� ȣ��
}

void ATFHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController=GetOwningPlayerController(); //���� �÷��̾� ��Ʈ�ѷ��� ������
	if (PlayerController && CharacterOverlayClass) //�÷��̾� ��Ʈ�ѷ��� ��ȿ�ϰ� ĳ���� �������� Ŭ������ �����Ǿ� �ִٸ�
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass); //ĳ���� �������� ������ ����
		CharacterOverlay->AddToViewport(); //������ ȭ�鿡 �߰�
	}
}

void ATFHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX(); //ũ�ν���� �ؽ����� �ʺ�
	const float TextureHeight = Texture->GetSizeY(); //ũ�ν���� �ؽ����� ����
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / ViewportDiv) + Spread.X,
		ViewportCenter.Y - (TextureHeight / ViewportDiv)+ Spread.Y); //ũ�ν���� �ؽ��ĸ� �׸� ��ġ ���
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, CrosshairColor); //ũ�ν���� �ؽ��ĸ� ȭ�鿡 �׸���
}
