// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Net/UnrealNetwork.h"//네트워크 관련 헤더 파일 포함
#include "Animation/AnimationAsset.h"//애니메이션 자산 관련 헤더 파일 포함
#include "Components/SkeletalMeshComponent.h"
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;//이 객체가 네트워크에서 복제 가능하도록 설정
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));//스켈레탈 메시 컴포넌트 생성
	WeaponMesh->SetupAttachment(RootComponent);//루트 컴포넌트에 부착
	SetRootComponent(WeaponMesh);//루트 컴포넌트 설정

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//모든 채널에 대해 충돌 응답을 차단
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//Pawn 채널에 대해 충돌 응답을 무시
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//충돌 비활성화

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));//스피어 컴포넌트 생성
	AreaSphere->SetupAttachment(RootComponent);//루트 컴포넌트에 부착
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//모든 채널에 대해 충돌 응답을 무시
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//충돌 비활성화

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));//위젯 컴포넌트 생성
	PickupWidget->SetupAttachment(RootComponent);//루트 컴포넌트에 부착
}

void AWeapon::ShowPickupWidget(bool bShowPickupWidget)
{
	if (PickupWidget) {
		PickupWidget->SetVisibility(bShowPickupWidget);//위젯의 가시성 설정
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWeapon, WeaonState);//무기 상태를 복제
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation) {
		WeaponMesh->PlayAnimation(FireAnimation, false);//발사 애니메이션 재생
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (PickupWidget) {//위젯이 존재하는 경우
		PickupWidget->SetVisibility(false);//위젯 숨기기
	}
	if (HasAuthority()) {//서버에서만 실행
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//충돌 활성화
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);//Pawn 채널에 대해 충돌 응답을 겹침으로 설정
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);//스피어 겹침 이벤트에 함수 바인딩
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnShpereEndOverlap);//스피어 겹침 종료 이벤트에 함수 바인딩
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ATimeFractureCharacter* Character = Cast<ATimeFractureCharacter>(OtherActor);//겹친 액터가 캐릭터인지 확인
	if (Character) {
		Character->SetOverlappingWeapon(this);//캐릭터에 겹치는 무기 설정
		
	}
}

void AWeapon::OnShpereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATimeFractureCharacter* Character = Cast<ATimeFractureCharacter>(OtherActor);//겹친 액터가 캐릭터인지 확인
	if (Character) {
		Character->SetOverlappingWeapon(nullptr);//캐릭터에 겹치는 무기 해제
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaonState = State;//무기 상태 설정
	switch (WeaonState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);//무기 위젯 숨기기
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//스피어 충돌 비활성화
		break;
	}
	
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaonState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);//장착된 무기 위젯 숨기기
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//스피어 충돌 비활성화
		break;
	case EWeaponState::EWS_Dropped:
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

