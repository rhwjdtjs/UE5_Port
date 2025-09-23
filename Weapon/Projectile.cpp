// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "UnrealProject_7A/UnrealProject_7A.h"
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // 이 액터가 네트워크에서 복제 가능하도록 설정
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkelatalMesh, ECollisionResponse::ECR_Block);
	
}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactNiagara,
			GetActorLocation(),GetActorRotation()
		);
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			GetActorLocation()
		);
	}
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}
void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnTrailSystem(); // 발사체의 트레일 시스템을 생성
	
	if (HasAuthority()) {
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit); // 서버에서만 Hit 이벤트를 바인딩
	}
	
	CollisionBox->IgnoreActorWhenMoving(GetInstigator(), true); // 발사체가 발사자를 무시하도록 설정
}


void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
//	ATimeFractureCharacter* TFCharacter = Cast<ATimeFractureCharacter>(OtherActor);
//	if (TFCharacter) {
//		TFCharacter->MultiCastHit(); // 플레이어 캐릭터가 맞았을 때 히트 리액트 몽타주 재생
//	}
	MulticastImpactEffect(Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	Destroy(); // 충돌 후 발사체를 파괴
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::SpawnTrailSystem()
{
	if (TracerNiagara) {
		TracerNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TracerNiagara,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			true  // bAutoDestroy
		);
	}
}

void AProjectile::ExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority()) {
		AController* FiringController = FiringPawn->GetController();
		if (FiringController) {
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, Damage, 10.f, GetActorLocation(), 200.f, 500.f, 1.f, UDamageType::StaticClass(), TArray<AActor*>(), this, FiringController); // 적	에게 피해를 적용
		}
	}
}

void AProjectile::MulticastImpactEffect_Implementation(const FVector& Location, const FRotator& Rotation)
{
	FRotator SpawnRotation = GetActorForwardVector().Rotation();
	if (ImpactNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ImpactNiagara,
			Location,
			SpawnRotation
		);
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			Location
		);
	}
}

