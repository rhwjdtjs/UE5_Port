// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "UnrealProject_7A/TFComponents/CBComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
void AAmmoPickup::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult); //부모 클래스의 OnSphereOverlap 함수를 호출한다.

	ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(OtherActor); //OtherActor를 ATimeFractureCharacter 타입으로 캐스팅한다.
	if (TFCharacter) {
		UCBComponent* CBComp = TFCharacter->GetCombatComponent();//캐릭터의 전투 컴포넌트를 가져온다.
		if (CBComp) {
			CBComp->PickupAmmo(WeaponType, AmmoAmount);//전투 컴포넌트의 PickupAmmo 함수를 호출하여 탄약을 추가한다.
		}
	}
	Destroy();//탄약 픽업 액터를 파괴한다.
}

