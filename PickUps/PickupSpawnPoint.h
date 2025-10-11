// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

/**
 * APickupSpawnPoint
 *
 * ������(Pickup) ���� �� �������� ����ϴ� ���� ����Ʈ Ŭ����.
 * - ������ �ð� �������� Pickup�� �����Ѵ�.
 * - ������ Pickup�� �ı��Ǹ� ���� �ð� �� �� Pickup�� �ٽ� �����Ѵ�.
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

	// ���� ������ Pickup Ŭ���� ���
	UPROPERTY(EditAnywhere, Category = "Pickup")
	TArray<TSubclassOf<class APickup>> PickupClasses;

	// ���� ������ Pickup �ν��Ͻ�
	UPROPERTY()
	APickup* SpawnedPickup;

	// Pickup ���� �Լ�
	void SpawnPickup();

	// Pickup Ÿ�̸� �Ϸ� �� ȣ��
	void SpawnPickupTimerFinished();

	// Pickup�� �ı��� �� ȣ��Ǿ� Ÿ�̸� �����
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

private:
	// Pickup ����� Ÿ�̸� �ڵ�
	FTimerHandle SpawnPickupTimer;

	// ���� ���� ���� (�ּ�~�ִ�)
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float SpawnPickupTimeMin;

	UPROPERTY(EditAnywhere, Category = "Pickup")
	float SpawnPickupTimeMax;
};
