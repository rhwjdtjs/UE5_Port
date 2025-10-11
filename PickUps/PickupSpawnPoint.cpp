// Fill out your copyright notice in the Description page of Project Settings.

#include "PickupSpawnPoint.h"
#include "Pickup.h"

/////////////////////////////////////////////////////////////
// 기능:
//   생성자. 기본값과 네트워크 복제를 설정한다.
//
// 알고리즘:
//   - Tick() 활성화: 매 프레임마다 갱신 가능하도록 설정.
//   - bReplicates = true: 서버에서 생성된 Pickup이 모든 클라이언트에 동기화되도록 설정.
/////////////////////////////////////////////////////////////
APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

/////////////////////////////////////////////////////////////
// 기능:
//   게임이 시작될 때 첫 Pickup 스폰 타이머를 시작한다.
//
// 알고리즘:
//   - StartSpawnPickupTimer(nullptr) 호출로 타이머를 바로 시작.
//   - 일정 시간이 지나면 SpawnPickupTimerFinished() → SpawnPickup() 순으로 호출됨.
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
// 기능:
//   실제 Pickup 액터를 생성한다.
//
// 알고리즘:
//   1. PickupClasses 배열에서 랜덤으로 클래스 선택.
//      - FMath::RandRange(0, Num-1)을 사용해 무작위 인덱스 선택.
//   2. GetWorld()->SpawnActor<APickup>()로 해당 클래스 인스턴스 생성.
//   3. 서버(HasAuthority())에서만 OnDestroyed 델리게이트 등록.
//      - Pickup이 파괴되면 StartSpawnPickupTimer() 자동 호출되어 리스폰 타이머 시작.
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
// 기능:
//   Pickup 타이머가 완료되면 새로운 Pickup을 스폰한다.
//
// 알고리즘:
//   - 서버(HasAuthority())에서만 SpawnPickup() 실행.
//   - 타이머가 끝나는 시점마다 새로운 아이템이 생성된다.
/////////////////////////////////////////////////////////////
void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

/////////////////////////////////////////////////////////////
// 기능:
//   Pickup이 파괴될 때 호출되어 일정 시간 후 리스폰 타이머를 시작한다.
//
// 알고리즘:
//   1. FMath::RandRange()로 최소~최대 범위 내에서 랜덤 시간 계산.
//   2. GetWorldTimerManager().SetTimer()로 타이머 시작.
//   3. 시간이 지나면 SpawnPickupTimerFinished() 호출.
//   4. 이렇게 “랜덤 딜레이 리스폰 시스템”을 구현함.
/////////////////////////////////////////////////////////////
void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::RandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &APickupSpawnPoint::SpawnPickupTimerFinished, SpawnTime);
}
