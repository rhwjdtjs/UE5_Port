// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickup.h"

#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/TFComponents/BuffComponent.h"


AShieldPickup::AShieldPickup()
{
	bReplicates = true;
}

void AShieldPickup::OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult); //�θ� Ŭ������ OnSphereOverlap �Լ��� ȣ���Ѵ�.
	UE_LOG(LogTemp, Warning, TEXT("AmmoPickup::OnSphereOverlap")); //�α� �޽����� ����Ѵ�.
	ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(OtherActor); //OtherActor�� ATimeFractureCharacter Ÿ������ ĳ�����Ѵ�.
	if (TFCharacter) {
		UBuffComponent* Buff = TFCharacter->GetBuffComponent(); //ĳ������ BuffComponent�� �����´�.
		if (Buff) {
			Buff->Shield(ShieldAmount, ShieldTime); //BuffComponent�� Shield �Լ��� ȣ���Ѵ�.
		}
	}
	Destroy();//ź�� �Ⱦ� ���͸� �ı��Ѵ�.
}