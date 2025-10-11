// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

/**
 * APickup
 *
 * �ʵ忡 ��ġ�Ǵ� ������(����, ź�� ��)�� ��Ÿ���� Ŭ����.
 * - �÷��̾ �����ϸ� ���� �̺�Ʈ�� �߻���Ų��.
 * - ȸ��, ����, ����Ʈ, ��Ʈ��ũ ���� ����� �����Ѵ�.
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

	// �÷��̾ ���Ǿ� ������ �������� �� ȣ��Ǵ� �Լ�
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// ȸ�� �ӵ�
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

	// ������ ���� ����
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	// ���� ����
	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	// ������ �޽�
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* PickupMesh;

	// ���̾ư��� ������Ʈ (�������� ȿ��)
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	// ���� �� ����Ǵ� ���̾ư��� ����Ʈ
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

private:
	// ��� Ŭ���̾�Ʈ�� ����Ʈ ����� �����ϴ� RPC
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayPickupEffects();

	// �ߺ� ���� ���� �÷���
	UPROPERTY()
	bool bPickedUp = false;

	// ���� �ð� �Ŀ� �浹 ���ε��� Ȱ��ȭ�ϱ� ���� Ÿ�̸�
	FTimerHandle BindOverlapTimer;
	float BindoverlapTime = 0.2f;
	void BindOverlapTimerFinished();

};
