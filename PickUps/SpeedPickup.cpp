// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/TFComponents/BuffComponent.h"


void ASpeedPickup::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult); //부모 클래스의 OnSphereOverlap 함수를 호출한다.
	UE_LOG(LogTemp, Warning, TEXT("AmmoPickup::OnSphereOverlap")); //로그 메시지를 출력한다.
	ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(OtherActor); //OtherActor를 ATimeFractureCharacter 타입으로 캐스팅한다.
	if (TFCharacter) {
		UBuffComponent* Buff = TFCharacter->GetBuffComponent(); //캐릭터의 BuffComponent를 가져온다.
		if (Buff) {
			Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime); //BuffComponent의 BuffSpeed 함수를 호출한다.
		}
	}
	Destroy();//탄약 픽업 액터를 파괴한다.
}