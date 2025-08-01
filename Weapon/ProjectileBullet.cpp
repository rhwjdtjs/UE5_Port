// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()); // 발사체의 소유자가 ACharacter인지 확인
	if (OwnerCharacter) { // 소유자가 ACharacter인 경우에만 피해를 적용
		AController* OwnerController = OwnerCharacter->Controller; // 소유자의 컨트롤러를 가져옴
		if (OwnerController) { // 컨트롤러가 유효한 경우에만 피해를 적용
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass()); // 피해를 적용
			}
		}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
