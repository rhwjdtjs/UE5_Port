// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Net/UnrealNetwork.h"//네트워크 관련 헤더 파일 포함
#include "Animation/AnimationAsset.h"//애니메이션 자산 관련 헤더 파일 포함
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"//케이싱 클래스 포함
#include "Engine/SkeletalMeshSocket.h"
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
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
	if (CasingClass) {
		const USkeletalMeshSocket* AmmoEjectSockt = WeaponMesh->GetSocketByName(FName("AmmoEject")); //무기 메시에서 "muzz" 소켓을 가져옴
		if (AmmoEjectSockt) {
			FTransform SocketTransform = AmmoEjectSockt->GetSocketTransform(GetWeaponMesh()); //소켓의 변환 정보를 가져옴
				UWorld* World = GetWorld();
				if (World) {
					World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()); //발사체를 생성
			}
		}
	}
}

void AWeapon::DropWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);//무기 상태를 떨어뜨린 상태로 설정
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);//무기의 소유자를 해제
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
		WeaponMesh->SetSimulatePhysics(false);//무기 메시 물리 시뮬레이션 활성화
		WeaponMesh->SetEnableGravity(false);//무기 메시 중력 활성화
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//무기 메시 충돌 활성화
		break;

	case EWeaponState::EWS_Dropped:
		if (HasAuthority()) {
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//스피어 충돌 활성화
		}
		WeaponMesh->SetSimulatePhysics(true);//무기 메시 물리 시뮬레이션 활성화
		WeaponMesh->SetEnableGravity(true);//무기 메시 중력 활성화
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//무기 메시 충돌 활성화
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
		WeaponMesh->SetSimulatePhysics(false);//무기 메시 물리 시뮬레이션 활성화
		WeaponMesh->SetEnableGravity(false);//무기 메시 중력 활성화
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//무기 메시 충돌 활성화
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);//무기 메시 물리 시뮬레이션 활성화
		WeaponMesh->SetEnableGravity(true);//무기 메시 중력 활성화
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//무기 메시 충돌 활성화
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

