// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
void AAmmoPickup::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult); //�θ� Ŭ������ OnSphereOverlap �Լ��� ȣ���Ѵ�.
	UE_LOG(LogTemp, Warning, TEXT("AmmoPickup::OnSphereOverlap")); //�α� �޽����� ����Ѵ�.
	ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(OtherActor); //OtherActor�� ATimeFractureCharacter Ÿ������ ĳ�����Ѵ�.
	if (TFCharacter) {
		UCBComponent* CBComp = TFCharacter->GetCombatComponent();//ĳ������ ���� ������Ʈ�� �����´�.
		if (CBComp) {
			CBComp->PickupAmmo(WeaponType, AmmoAmount);//���� ������Ʈ�� PickupAmmo �Լ��� ȣ���Ͽ� ź���� �߰��Ѵ�.
		}
		if (ATFPlayerController* PC = Cast<ATFPlayerController>(TFCharacter->Controller))
		{
			PC->ClientPlayPickupEffects(PickupSound, PickupEffect, GetActorLocation(), GetActorRotation());
		}
	}
}

