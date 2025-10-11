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
#include "UnrealProject_7A/PlayerController/TFPlayerController.h"//�÷��̾� ��Ʈ�ѷ� Ŭ���� ����
#include "UnrealProject_7A/TFComponents/CBComponent.h"

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

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_A);//����� ���� �� ���ٽ� �� ����
	WeaponMesh->MarkRenderStateDirty();//���� ���¸� �������� ������Ʈ �ʿ�
	EnableCustomDepth(true);//����� ���� �� Ȱ��ȭ
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));//���Ǿ� ������Ʈ ����
	AreaSphere->SetupAttachment(RootComponent);//��Ʈ ������Ʈ�� ����
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//��� ä�ο� ���� �浹 ������ ����
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�浹 ��Ȱ��ȭ

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));//���� ������Ʈ ����
	PickupWidget->SetupAttachment(RootComponent);//��Ʈ ������Ʈ�� ����
}
void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh) {
		WeaponMesh->SetRenderCustomDepth(bEnable);//���� �޽��� ����� ���� ���� ����
	}
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
	DOREPLIFETIME(AWeapon, Ammo);//ź���� ����
}

void AWeapon::SetHUDAmmo()
{
	TFOwnerCharacter = TFOwnerCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetOwner()) : TFOwnerCharacter; //�����ڰ� ĳ�������� Ȯ��
	if (TFOwnerCharacter) {
		TFOwnerController = TFOwnerController == nullptr ? Cast<ATFPlayerController>(TFOwnerCharacter->Controller) : TFOwnerController; //�������� ��Ʈ�ѷ��� �÷��̾� ��Ʈ�ѷ����� Ȯ��
		if (TFOwnerController) {
			TFOwnerController->SetHUDWeaponAmmo(Ammo); //HUD�� ���� ź���� ������Ʈ
		}
	}
}

void AWeapon::SpendRound() {
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity); //ź�� ���� �� ���� ����
	SetHUDAmmo();
}
void AWeapon::OnRep_Ammo() {
	TFOwnerCharacter = TFOwnerCharacter == nullptr ? Cast<ATimeFractureCharacter>(GetOwner()) : TFOwnerCharacter; //�����ڰ� ĳ�������� Ȯ��
	if(TFOwnerCharacter && TFOwnerCharacter->GetCombatComponent() && IsFull()) {
		TFOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd(); //������ ��� �ִϸ��̼��� ������ �̵�
	}
	SetHUDAmmo();
}
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr) {
		TFOwnerCharacter = nullptr; //������ ĳ���͸� nullptr�� ����
		TFOwnerController = nullptr;//������ ĳ���Ϳ� �÷��̾� ��Ʈ�ѷ��� nullptr�� ����
	}
	else {
		TFOwnerCharacter = TFOwnerCharacter == nullptr ? Cast<ATimeFractureCharacter>(Owner) : TFOwnerCharacter; //�����ڰ� ĳ�������� Ȯ��
		if (TFOwnerCharacter && TFOwnerCharacter->GetEquippedWeapon() && TFOwnerCharacter->GetEquippedWeapon() == this) {
			SetHUDAmmo();//�����ڰ� ����Ǹ� HUD�� ź���� ������Ʈ
		}
	}
	
}
bool AWeapon::IsEmpty()
{
	return Ammo <= 0; //ź���� 0 ������ ��� true ��ȯ
}
bool AWeapon::IsFull()
{
	return Ammo == MagCapacity; //ź���� źâ �뷮�� ���� ��� true ��ȯ
}
// ============================================================
// [�߻� ó��] Fire()
// ------------------------------------------------------------
// ��� ��� : 
//   - �߻� �ִϸ��̼� ����, ź�� ����, ź�� ����.
// �˰��� ���� : 
//   1. FireAnimation ���
//   2. CasingClass�� AmmoEject ���� ��ġ���� Spawn
//   3. SpendRound() ȣ���Ͽ� ź�� ����
// ============================================================
void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation) {
		WeaponMesh->PlayAnimation(FireAnimation, false);//�߻� �ִϸ��̼� ���
	}
	if (CasingClass) {
		const USkeletalMeshSocket* AmmoEjectSockt = WeaponMesh->GetSocketByName(FName("AmmoEject")); //ź�� ���� ������ ������
		if (AmmoEjectSockt) {
			FTransform SocketTransform = AmmoEjectSockt->GetSocketTransform(GetWeaponMesh()); //������ ��ȯ ������ ������
				UWorld* World = GetWorld();
				if (World) {
					World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()); //�߻�ü�� ����
			}
		}
	}
	SpendRound();//ź�� ����
}



void AWeapon::DropWeapon()
{
	SetWeaponState(EWeaponState::EWS_Dropped);//���� ���¸� ����߸� ���·� ����
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);//������ �����ڸ� ����
	TFOwnerCharacter = nullptr; //������ ĳ���͸� nullptr�� ����
	TFOwnerController = nullptr;//������ ĳ���Ϳ� �÷��̾� ��Ʈ�ѷ��� nullptr�� ����
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity); //ź���� �߰��ϰ� ������ ����
	SetHUDAmmo(); //HUD�� ź�� ������Ʈ
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
	OnWeaponStateSet();//���� ���¿� ���� ���� ����
	
}
void AWeapon::OnWeaponStateSet()
{
	switch (WeaonState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();//���� ���� �� ���� ����
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();//���� ���� ���� �� ���� ����
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();//���� ����߸� �� ���� ����
		break;
	}
}
void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}
void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);//���� ���� �����
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//���Ǿ� �浹 ��Ȱ��ȭ
	WeaponMesh->SetSimulatePhysics(false);//���� �޽� ���� �ùķ��̼� Ȱ��ȭ
	WeaponMesh->SetEnableGravity(false);//���� �޽� �߷� Ȱ��ȭ
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//���� �޽� �浹 Ȱ��ȭ
	EnableCustomDepth(false); //������ ������ Ŀ���� ���̸� ��Ȱ��ȭ�Ѵ�.
}
void AWeapon::OnDropped()
{
	if (HasAuthority()) {
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//���Ǿ� �浹 Ȱ��ȭ
	}
	WeaponMesh->SetSimulatePhysics(true);//���� �޽� ���� �ùķ��̼� Ȱ��ȭ
	WeaponMesh->SetEnableGravity(true);//���� �޽� �߷� Ȱ��ȭ
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//���� �޽� �浹 Ȱ��ȭ
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_A);//����� ���� �� ���ٽ� �� ����
	WeaponMesh->MarkRenderStateDirty();//���� ���¸� �������� ������Ʈ �ʿ�
	EnableCustomDepth(true);//����� ���� �� Ȱ��ȭ
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);//���� ���� �����
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//���Ǿ� �浹 ��Ȱ��ȭ
	WeaponMesh->SetSimulatePhysics(false);//���� �޽� ���� �ùķ��̼� Ȱ��ȭ
	WeaponMesh->SetEnableGravity(false);//���� �޽� �߷� Ȱ��ȭ
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//���� �޽� �浹 Ȱ��ȭ
	EnableCustomDepth(true); //������ ������ Ŀ���� ���̸� ��Ȱ��ȭ�Ѵ�.
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_C);//����� ���� �� ���ٽ� �� ����
		WeaponMesh->MarkRenderStateDirty();//���� ���¸� �������� ������Ʈ �ʿ�
	}
}


