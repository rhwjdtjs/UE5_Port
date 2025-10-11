// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

/**
 * APickup
 *
 * 필드에 배치되는 아이템(무기, 탄약 등)을 나타내는 클래스.
 * - 플레이어가 접근하면 습득 이벤트를 발생시킨다.
 * - 회전, 사운드, 이펙트, 네트워크 복제 기능을 포함한다.
 */
UCLASS()
class UNREALPROJECT_7A_API APickup : public AActor
{
	GENERATED_BODY()

public:
	APickup();

	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	// 플레이어가 스피어 범위에 진입했을 때 호출되는 함수
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// 회전 속도
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

	// 아이템 감지 범위
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	// 습득 사운드
	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	// 아이템 메시
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* PickupMesh;

	// 나이아가라 컴포넌트 (지속적인 효과)
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	// 습득 시 재생되는 나이아가라 이펙트
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

private:
	// 모든 클라이언트에 이펙트 재생을 전파하는 RPC
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayPickupEffects();

	// 중복 습득 방지 플래그
	UPROPERTY()
	bool bPickedUp = false;

	// 일정 시간 후에 충돌 바인딩을 활성화하기 위한 타이머
	FTimerHandle BindOverlapTimer;
	float BindoverlapTime = 0.2f;
	void BindOverlapTimerFinished();

};
