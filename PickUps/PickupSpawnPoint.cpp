// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupSpawnPoint.h"
#include "Pickup.h"

/////////////////////////////////////////////////////////////
// ���:
//   ������. �⺻���� ��Ʈ��ũ ������ �����Ѵ�.
//
// �˰���:
//   - Tick() Ȱ��ȭ: �� �����Ӹ��� ���� �����ϵ��� ����.
//   - bReplicates = true: �������� ������ Pickup�� ��� Ŭ���̾�Ʈ�� ����ȭ�ǵ��� ����.
/////////////////////////////////////////////////////////////
APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

/////////////////////////////////////////////////////////////
// ���:
//   ������ ���۵� �� ù Pickup ���� Ÿ�̸Ӹ� �����Ѵ�.
//
// �˰���:
//   - StartSpawnPickupTimer(nullptr) ȣ��� Ÿ�̸Ӹ� �ٷ� ����.
//   - ���� �ð��� ������ SpawnPickupTimerFinished() �� SpawnPickup() ������ ȣ���.
/////////////////////////////////////////////////////////////
void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickupTimer((AActor*)nullptr);
}
void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/////////////////////////////////////////////////////////////
// ���:
//   ���� Pickup ���͸� �����Ѵ�.
//
// �˰���:
//   1. PickupClasses �迭���� �������� Ŭ���� ����.
//      - FMath::RandRange(0, Num-1)�� ����� ������ �ε��� ����.
//   2. GetWorld()->SpawnActor<APickup>()�� �ش� Ŭ���� �ν��Ͻ� ����.
//   3. ����(HasAuthority())������ OnDestroyed ��������Ʈ ���.
//      - Pickup�� �ı��Ǹ� StartSpawnPickupTimer() �ڵ� ȣ��Ǿ� ������ Ÿ�̸� ����.
/////////////////////////////////////////////////////////////
void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		if (HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}
}

/////////////////////////////////////////////////////////////
// ���:
//   Pickup Ÿ�̸Ӱ� �Ϸ�Ǹ� ���ο� Pickup�� �����Ѵ�.
//
// �˰���:
//   - ����(HasAuthority())������ SpawnPickup() ����.
//   - Ÿ�̸Ӱ� ������ �������� ���ο� �������� �����ȴ�.
/////////////////////////////////////////////////////////////
void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

/////////////////////////////////////////////////////////////
// ���:
//   Pickup�� �ı��� �� ȣ��Ǿ� ���� �ð� �� ������ Ÿ�̸Ӹ� �����Ѵ�.
//
// �˰���:
//   1. FMath::RandRange()�� �ּ�~�ִ� ���� ������ ���� �ð� ���.
//   2. GetWorldTimerManager().SetTimer()�� Ÿ�̸� ����.
//   3. �ð��� ������ SpawnPickupTimerFinished() ȣ��.
//   4. �̷��� ������ ������ ������ �ý��ۡ��� ������.
/////////////////////////////////////////////////////////////
void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::RandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &APickupSpawnPoint::SpawnPickupTimerFinished, SpawnTime);
}
