// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

/**
 * APickupSpawnPoint
 *
 * 아이템(Pickup) 생성 및 리스폰을 담당하는 스폰 포인트 클래스.
 * - 랜덤한 시간 간격으로 Pickup을 스폰한다.
 * - 스폰된 Pickup이 파괴되면 일정 시간 후 새 Pickup을 다시 생성한다.
 */
UCLASS()
class UNREALPROJECT_7A_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	// 생성 가능한 Pickup 클래스 목록
	UPROPERTY(EditAnywhere, Category = "Pickup")
	TArray<TSubclassOf<class APickup>> PickupClasses;

	// 현재 스폰된 Pickup 인스턴스
	UPROPERTY()
	APickup* SpawnedPickup;

	// Pickup 생성 함수
	void SpawnPickup();

	// Pickup 타이머 완료 시 호출
	void SpawnPickupTimerFinished();

	// Pickup이 파괴될 때 호출되어 타이머 재시작
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

private:
	// Pickup 재생성 타이머 핸들
	FTimerHandle SpawnPickupTimer;

	// 랜덤 스폰 간격 (최소~최대)
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float SpawnPickupTimeMin;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	float SpawnPickupTimeMax;
};
