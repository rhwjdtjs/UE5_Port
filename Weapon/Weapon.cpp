// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Net/UnrealNetwork.h"//��Ʈ��ũ ���� ��� ���� ����
#include "Animation/AnimationAsset.h"//�ִϸ��̼� �ڻ� ���� ��� ���� ����
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"//���̽� Ŭ���� ����
#include "Engine/SkeletalMeshSocket.h"
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;//�� ��ü�� ��Ʈ��ũ���� ���� �����ϵ��� ����
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));//���̷�Ż �޽� ������Ʈ ����
	WeaponMesh->SetupAttachment(RootComponent);//��Ʈ ������Ʈ�� ����
	SetRootComponent(WeaponMesh);//��Ʈ ������Ʈ ����

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//��� ä�ο� ���� �浹 ������ ����
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//Pawn ä�ο� ���� �浹 ������ ����
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�浹 ��Ȱ��ȭ

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));//���Ǿ� ������Ʈ ����
	AreaSphere->SetupAttachment(RootComponent);//��Ʈ ������Ʈ�� ����
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//��� ä�ο� ���� �浹 ������ ����
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�浹 ��Ȱ��ȭ

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));//���� ������Ʈ ����
	PickupWidget->SetupAttachment(RootComponent);//��Ʈ ������Ʈ�� ����
}

void AWeapon::ShowPickupWidget(bool bShowPickupWidget)
{
	if (PickupWidget) {
		PickupWidget->SetVisibility(bShowPickupWidget);//������ ���ü� ����
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWeapon, WeaonState);//���� ���¸� ����
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation) {
		WeaponMesh->PlayAnimation(FireAnimation, false);//�߻� �ִϸ��̼� ���
	}
	if (CasingClass) {
		const USkeletalMeshSocket* AmmoEjectSockt = WeaponMesh->GetSocketByName(FName("AmmoEject")); //���� �޽ÿ��� "muzz" ������ ������
		if (AmmoEjectSockt) {
			FTransform SocketTransform = AmmoEjectSockt->GetSocketTransform(GetWeaponMesh()); //������ ��ȯ ������ ������
				UWorld* World = GetWorld();
				if (World) {
					World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()); //�߻�ü�� ����
			}
		}
	}
}

void AWeapon::DropWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);//���� ���¸� ����߸� ���·� ����
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);//������ �����ڸ� ����
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (PickupWidget) {//������ �����ϴ� ���
		PickupWidget->SetVisibility(false);//���� �����
	}
	if (HasAuthority()) {//���������� ����
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//�浹 Ȱ��ȭ
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);//Pawn ä�ο� ���� �浹 ������ ��ħ���� ����
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);//���Ǿ� ��ħ �̺�Ʈ�� �Լ� ���ε�
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnShpereEndOverlap);//���Ǿ� ��ħ ���� �̺�Ʈ�� �Լ� ���ε�
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ATimeFractureCharacter* Character = Cast<ATimeFractureCharacter>(OtherActor);//��ģ ���Ͱ� ĳ�������� Ȯ��
	if (Character) {
		Character->SetOverlappingWeapon(this);//ĳ���Ϳ� ��ġ�� ���� ����
		
	}
}

void AWeapon::OnShpereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ATimeFractureCharacter* Character = Cast<ATimeFractureCharacter>(OtherActor);//��ģ ���Ͱ� ĳ�������� Ȯ��
	if (Character) {
		Character->SetOverlappingWeapon(nullptr);//ĳ���Ϳ� ��ġ�� ���� ����
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaonState = State;//���� ���� ����
	switch (WeaonState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);//���� ���� �����
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//���Ǿ� �浹 ��Ȱ��ȭ
		WeaponMesh->SetSimulatePhysics(false);//���� �޽� ���� �ùķ��̼� Ȱ��ȭ
		WeaponMesh->SetEnableGravity(false);//���� �޽� �߷� Ȱ��ȭ
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//���� �޽� �浹 Ȱ��ȭ
		break;

	case EWeaponState::EWS_Dropped:
		if (HasAuthority()) {
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//���Ǿ� �浹 Ȱ��ȭ
		}
		WeaponMesh->SetSimulatePhysics(true);//���� �޽� ���� �ùķ��̼� Ȱ��ȭ
		WeaponMesh->SetEnableGravity(true);//���� �޽� �߷� Ȱ��ȭ
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//���� �޽� �浹 Ȱ��ȭ
		break;
	}
	
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaonState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);//������ ���� ���� �����
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//���Ǿ� �浹 ��Ȱ��ȭ
		WeaponMesh->SetSimulatePhysics(false);//���� �޽� ���� �ùķ��̼� Ȱ��ȭ
		WeaponMesh->SetEnableGravity(false);//���� �޽� �߷� Ȱ��ȭ
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//���� �޽� �浹 Ȱ��ȭ
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);//���� �޽� ���� �ùķ��̼� Ȱ��ȭ
		WeaponMesh->SetEnableGravity(true);//���� �޽� �߷� Ȱ��ȭ
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//���� �޽� �浹 Ȱ��ȭ
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

