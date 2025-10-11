

#include "CBComponent.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" //������ ���� ��������� ���Խ�Ų��.
#include "GameFramework/CharacterMovementComponent.h" //ĳ���� �̵� ������Ʈ�� ���Խ�Ų��.
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "UnrealProject_7a/PlayerController/TFPlayerController.h"
#include "UnrealProject_7a/HUD/TFHUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "CombatStates.h"
#include "UnrealProject_7A/UnrealProject_7A.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Sound/SoundCue.h"
#include "UnrealProject_7A/Character/TFAnimInstance.h"
#include "UnrealProject_7A/Weapon/Projectile.h"
#include "Components/BoxComponent.h"
#include "UnrealProject_7A/PlayerState/TFPlayerState.h"

UCBComponent::UCBComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}
// ============================================================
// [���� ����] EquipWeapon()
// ------------------------------------------------------------
// ��� ��� : 
//   - ĳ���Ͱ� ���ο� ���⸦ �����Ѵ�.
//   - �� ���� / ���� ���⸦ �����Ͽ� �ùٸ� ���Կ� ����.
// �˰��� ���� : 
//   1. ĳ���� �Ǵ� ���Ⱑ ������ ����
//   2. ���� ���� ���°� ������� ������ ���� �Ұ�
//   3. ������ ���Ⱑ �ִٸ� �������� �������� �̵�
//   4. ���ٸ� �ֹ���� ����
//   5. ĳ������ Yaw ȸ�� ���� Ȱ��ȭ
// ============================================================
void UCBComponent::EquipWeapon(AWeapon* WeaponEquip)
{
	if (Character == nullptr || WeaponEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied)return; //���� ���°� ������� ������ �������� �ʴ´�.
	if(EquippedWeapon !=nullptr && SecondaryWeapon==nullptr) //������ ���Ⱑ �ְ� ���� ���Ⱑ ������
	{
		EquipSecondaryWeapon(WeaponEquip); //���� ���⸦ �����Ѵ�.
	}
	else
	{
		EquipPrimaryWeapon(WeaponEquip); //�� ���⸦ �����Ѵ�.
	}

	Character->bUseControllerRotationYaw = true; //ĳ���Ͱ� ��Ʈ�ѷ��� Yaw ȸ���� ����ϵ��� �����Ѵ�.
	
}
// ============================================================
// [���� ��ü] SwapWeapons()
// ------------------------------------------------------------
// ��� ��� : 
//   - �ֹ���� ���������� ������ ���� ��ü�Ѵ�.
//   - HUD ź�� ���� ���� �� ���� ��� ����.
// �˰��� ���� : 
//   1. ���ε� ���̸� ��ü �Ұ�
//   2. EquippedWeapon, SecondaryWeapon�� ���� ��ȯ
//   3. �ֹ���� ����������, ��������� ������ �̵�
//   4. HUD �� ź�� ��, ���� ������Ʈ
// ============================================================
void UCBComponent::SwapWeapons()
{
	if (CombatState == ECombatState::ECS_Reloading) return;
	AWeapon* TempWeapon = EquippedWeapon; //������ ���⸦ �ӽ� ������ �����Ѵ�.
	EquippedWeapon = SecondaryWeapon; //������ ���⿡ ���� ���⸦ �����Ѵ�.
	SecondaryWeapon = TempWeapon; //���� ���⿡ �ӽ� ������ ����� ������ ���⸦ �����Ѵ�.

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeaponPositionModify();
	EquippedWeapon->SetHUDAmmo(); //������ ������ HUD ź���� �����Ѵ�.
	UpdateCarriedAmmo(); //���� ź���� ������Ʈ�Ѵ�.
	PlayEquipSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBack(SecondaryWeapon); //������ ���⸦ � �����Ѵ�.
}
// ============================================================
// [�ֹ��� ����] EquipPrimaryWeapon()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���ο� ���⸦ �ֹ��� ���Կ� �����Ѵ�.
// �˰��� ���� : 
//   1. ���� ���Ⱑ �ִٸ� Drop ó��
//   2. ���� ���¸� ��Equipped���� ����
//   3. ������ ���Ͽ� ����
//   4. ������ �� HUD ����
//   5. ���� ���� ��� �� ���� �浹 ��Ȱ��ȭ
// ============================================================
void UCBComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon(); //�̹� ������ ���Ⱑ ������ ����Ѵ�.
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeaponPositionModify();
	EquippedWeapon->SetHUDAmmo(); //������ ������ HUD ź���� �����Ѵ�.
	UpdateCarriedAmmo(); //���� ź���� ������Ʈ�Ѵ�.
	PlayEquipSound(WeaponToEquip);
	EquippedWeapon->ShowPickupWidget(false);
	EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ReloadEmptyWeapon();
}
// ============================================================
// [�������� ����] EquipSecondaryWeapon()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���⸦ �������� ���Կ� �����Ѵ�.
// �˰��� ���� : 
//   1. ���� ���¸� ��EquippedSecondary���� ����
//   2. ��(back socket)�� ����
//   3. ���� ��� �� ������ ����
// ============================================================
void UCBComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBack(WeaponToEquip); //������ ���⸦ � �����Ѵ�.
	PlayEquipSound(WeaponToEquip);
	SecondaryWeapon->SetOwner(Character);
}
// ============================================================
// [���� ���� ���� ����] OnRep_EquippedWeapon()
// ------------------------------------------------------------
// ��� ��� : 
//   - Ŭ���̾�Ʈ���� ���� ���� ������ �����Ǿ��� ��,
//     �ð���/����/ź�� �����͸� �ݿ��Ѵ�.
// ��� ��� : 
//   - �𸮾� ��Ʈ��ũ ���� (RepNotify)
//   - AttachToComponent() / HUD ����
// ============================================================
void UCBComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon) {
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false); //������ ������ Ŀ���� ���̸� ��Ȱ��ȭ�Ѵ�.
		EquippedWeapon->SetHUDAmmo(); //������ ������ HUD ź���� �����Ѵ�.
	}
	PlayEquipSound(EquippedWeapon);
}
// ============================================================
// [�������� ���� ����] OnRep_SecondaryWeapon()
// ------------------------------------------------------------
// ��� ��� : 
//   - Ŭ���̾�Ʈ���� �������� ������ ���ŵ� �� ȣ��ȴ�.
//   - ���⸦ ��(back socket)�� �����ϰ� ���� ���� ���.
// ============================================================
void UCBComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character) {
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBack(SecondaryWeapon); //������ ���⸦ � �����Ѵ�.
		PlayEquipSound(EquippedWeapon);

	}
}
// ============================================================
// [���� ���] DropEquippedWeapon()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���� ���� ���� ���⸦ �ٴڿ� ����߸���.
//   - ���� ����� ����/�浹�� �ٽ� Ȱ��ȭ�ȴ�.
// ============================================================
void UCBComponent::DropEquippedWeapon()
{
	if (EquippedWeapon) {
		EquippedWeapon->DropWeapon(); //�̹� ������ ���Ⱑ ������ ����Ѵ�.
	}
}
// ============================================================
// [���� ������ ����] AttachActorToRightHand()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���⸦ ĳ������ ������ ����("RightHandSocket")�� �����Ѵ�.
// �˰��� ���� : 
//   1. ĳ����, �޽�, ���� ��ȿ�� �˻�
//   2. FAttachmentTransformRules::SnapToTargetNotIncludingScale ���
// ============================================================
void UCBComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr|| Character->GetMesh()==nullptr || ActorToAttach == nullptr) return;
	ActorToAttach->AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("RightHandSocket")
	);
}
// ============================================================
// [���� �޼� ����] AttachActorToLeftHand()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���⸦ ĳ������ �޼� ����("LeftHandSocket")�� �����Ѵ�.
//   - ���ε峪 ����ź ��ô �� �ӽ� ��ġ ��������� ���.
// ============================================================
void UCBComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	ActorToAttach->AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("LeftHandSocket")
	);
}
// ============================================================
// [���� �� ����] AttachActorToBack()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���⸦ ĳ������ �� ����("BackWeaponSocket")�� �����Ѵ�.
//   - �������� ������ ��� ȸ��/��ġ ������ �߰� ����.
// ============================================================
void UCBComponent::AttachActorToBack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* BackSocket = Character->GetMesh()->GetSocketByName(FName("BackWeaponSocket"));
	if (BackSocket) {
		BackSocket->AttachActor(ActorToAttach, Character->GetMesh());
		if(EWeaponType::EWT_SniperRifle == SecondaryWeapon->GetWeaponType())
		{
			FVector FOffset = FVector(0.f, 0.f, 0.f);
			FRotator Offset = FRotator(0.f, 90.f, 0.f); // Z���� ���� 10��ŭ �ø���
			ActorToAttach->GetRootComponent()->SetRelativeRotation(Offset); //������ ��Ʈ ������Ʈ ��ġ�� �����Ѵ�.
			ActorToAttach->GetRootComponent()->SetRelativeLocation(FOffset); //������ ��Ʈ ������Ʈ ��ġ�� �����Ѵ�.
		}
	}
}
// ============================================================
// [���� ź�� ����] UpdateCarriedAmmo()
// ------------------------------------------------------------
// ��� ��� : 
//   - ������ ������ Ÿ�Կ� ���� ���� ź�� ���� HUD�� �ݿ��Ѵ�.
// �˰��� ���� : 
//   1. ������ ���� Ȯ�� �� CarriedAmmoMap���� �ش� Ÿ�� �˻�
//   2. ���� ź�� ���� Controller HUD�� ����
// ============================================================
void UCBComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr)return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; //������ ������ ���� ź���� �����Ѵ�.
	}
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //��Ʈ�ѷ��� �����´�.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //��Ʈ�ѷ��� �ִٸ� HUD�� ���� ź���� �����Ѵ�.
	}
}
void UCBComponent::PlayEquipSound(AWeapon* WeaponToEquip)
{
	if (Character&& WeaponToEquip && WeaponToEquip->EquipSound) {
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation()); //���� ���带 ����Ѵ�.
	}
}
void UCBComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon&&EquippedWeapon->IsEmpty()) {
		Reload(); //������ ���Ⱑ ��������� ���ε带 �õ��Ѵ�.
	}
}
// ============================================================
// [���� ��ġ ����] EquippedWeaponPositionModify()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���� ����(EWeaponType)�� ���� �տ� ����� ���� ��ġ/ȸ�� ����.
// �˰��� ���� : 
//   - ����: ��ġ�� �ణ ����
//   - ����: �ణ �Ʒ���
//   - �������� ������: ȸ�� + ��ġ ������ ����
// ============================================================
void UCBComponent::EquippedWeaponPositionModify()
{
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun)
	{
		FVector Offset = FVector(0.f, 0.f, 23.f); // Z���� ���� 10��ŭ �ø���
		EquippedWeapon->GetRootComponent()->SetRelativeLocation(Offset); //������ ��Ʈ ������Ʈ ��ġ�� �����Ѵ�.
	}
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol) {
		FVector Offset = FVector(0.f, 0.f, -5.f); // Z���� ���� 10��ŭ �ø���
		EquippedWeapon->GetRootComponent()->SetRelativeLocation(Offset); //������ ��Ʈ ������Ʈ ��ġ�� �����Ѵ�.
	}
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) {
		FVector FOffset = FVector(0.f, 40.f, 22.f);
		FRotator Offset = FRotator(0.f, 90.f, 0.f); // Z���� ���� 10��ŭ �ø���
		EquippedWeapon->GetRootComponent()->SetRelativeRotation(Offset); //������ ��Ʈ ������Ʈ ��ġ�� �����Ѵ�.
		EquippedWeapon->GetRootComponent()->SetRelativeLocation(FOffset); //������ ��Ʈ ������Ʈ ��ġ�� �����Ѵ�.
	}
}
// ============================================================
// [��Ʈ��ũ ���� ���] GetLifetimeReplicatedProps()
// ------------------------------------------------------------
// ��� ��� : 
//   - �𸮾��� DOREPLIFETIME ��ũ�η� ��Ʈ��ũ ���� ������ �����Ѵ�.
// ���� �׸� : 
//   - EquippedWeapon, SecondaryWeapon, bisAiming, CombatState, Grenades
//   - CarriedAmmo (������ ���� ����)
// ============================================================
void UCBComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCBComponent, EquippedWeapon); //������ ���⸦ �����Ѵ�.
	DOREPLIFETIME(UCBComponent, SecondaryWeapon); //���� ���⸦ �����Ѵ�.
	DOREPLIFETIME(UCBComponent, bisAiming);// ���� ���θ� �����Ѵ�.
	DOREPLIFETIME(UCBComponent, CombatState); //���� ���¸� �����Ѵ�.
	DOREPLIFETIME(UCBComponent, Grenades); //����ź ������ �����Ѵ�.
	DOREPLIFETIME_CONDITION(UCBComponent, CarriedAmmo, COND_OwnerOnly); // �����ڸ� ź���� �����Ѵ�.
	//DOREPLIFETIME_CONDITION(UCBComponent, EquippedWeapon, COND_OwnerOnly); //������ ���⸦ �����ϴµ�, ������ �����ڸ� �����Ѵٴ� ���̴�.
}
// ============================================================
// [���� ���� ���� ����] OnRep_CombatState()
// ------------------------------------------------------------
// ��� ��� : 
//   - �������� ���� ���°� ����Ǹ� Ŭ���̾�Ʈ ������ ���¸� ����ȭ�Ѵ�.
// �˰��� ���� : 
//   - Reloading �� ���ε� ó��
//   - ThrowingGrenade �� ����ź ��ô ��� �� �޼� ���� �̵�
//   - Unoccupied �� �߻� ��ư ���� �� �ڵ� �߻�
// ============================================================
void UCBComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed) {
			Fire(); //�߻� ��ư�� �������� �߻��Ѵ�.
		}
		break;
	case ECombatState::ECS_Reloading:
		UE_LOG(LogTemp, Warning, TEXT("Client: OnRep_CombatState - Reloading started"));
			HandleReload(); //���ε带 ó���Ѵ�.
		
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled()) {
			Character->PlayThrowGrendadeMontage(); //ĳ������ ����ź ������ ��Ÿ�ָ� ����Ѵ�.
			AttachActorToLeftHand(EquippedWeapon); //������ ���⸦ �޼տ� �����Ѵ�.
			ShowAttachedGrenade(true); //ĳ������ ������ ����ź�� ���̰� �Ѵ�.
		}
	case ECombatState::ECS_MAX:
		break;
	}
}

void UCBComponent::Reload() {
	if (CarriedAmmo > 0 && CombatState !=ECombatState::ECS_Reloading && EquippedWeapon && !EquippedWeapon->IsFull()) {
		ServerReload(); //������ ���ε� ��û�� ������.
	}
}

void UCBComponent::HandleReload()
{
	if (Character) {
		Character->PlayReloadMontage(); //ĳ������ ���ε� ��Ÿ�ָ� ����Ѵ�.
	}
}
// ============================================================
// [���ε� �� ���] AmountToReload()
// ------------------------------------------------------------
// ��� ��� : 
//   - źâ ���� ����(RoomInMag)�� ���� ź�� �� �ּҰ� ���.
//   - ���ε� ������ ���� ź�� �� ��ȯ.
// ============================================================
int32 UCBComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0; //������ ���Ⱑ ������ 0�� ��ȯ�Ѵ�.
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo(); //źâ�� ���� ������ ����Ѵ�.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) { // ������ ������ Ÿ���� ���� ź�� �ʿ� �ִ��� Ȯ��
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; // ������ ���� Ÿ���� ���� ���� ź�� ���� ������
		int32 Least = FMath::Min(RoomInMag, AmountCarried); // źâ�� ���� ������ ���� ź�� �� �� ���� ���� ����
		return FMath::Clamp(RoomInMag, 0, Least); // 0�� Least ������ ������ RoomInMag�� �����Ͽ� ��ȯ (�ִ� ���� ������ ź�� ��)
	}
	return 0;
}
// ============================================================
// [ź�� �� ������Ʈ] UpdateAmmoValues()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���ε� �� ���� ź���� �����ϰ�, źâ�� �߰��Ѵ�.
//   - HUD ź�� ǥ�ø� �����Ѵ�.
// ============================================================
void UCBComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return; //ĳ���Ͱ� ������ �Լ��� �����Ѵ�.
	int32 ReloadAmount = AmountToReload(); //���ε��� ���� ����Ѵ�.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) { // ������ ������ Ÿ���� ���� ź�� �ʿ� �ִ��� Ȯ��
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount; // ���� ź�࿡�� ���ε��� ���� �����Ѵ�.
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; //���� ���� ź���� ������Ʈ�Ѵ�.
	}
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //��Ʈ�ѷ��� �����´�.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //��Ʈ�ѷ��� �ִٸ� HUD�� ���� ź���� �����Ѵ�.
	}
	EquippedWeapon->AddAmmo(-ReloadAmount); //������ ���⿡ ���ε��� ���� �߰��Ѵ�.
}
void UCBComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return; //ĳ���Ͱ� ������ �Լ��� �����Ѵ�.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) { // ������ ������ Ÿ���� ���� ź�� �ʿ� �ִ��� Ȯ��
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1; // ���� ź�࿡�� ���ε��� ���� �����Ѵ�.
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; //���� ���� ź���� ������Ʈ�Ѵ�.
	}
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //��Ʈ�ѷ��� �����´�.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //��Ʈ�ѷ��� �ִٸ� HUD�� ���� ź���� �����Ѵ�.
	}
	EquippedWeapon->AddAmmo(-1); //������ ���⿡ ���ε��� ���� �߰��Ѵ�.
	bCanFire = true; //�߻� ���� ���θ� true�� �����Ѵ�.
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0) {
		JumpToShotgunEnd(); //������ ���Ⱑ ���� ���� ���� ������ �����Ѵ�.
	}
}
void UCBComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade()) {
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade); //ĳ������ ������ ����ź�� ���ü��� �����Ѵ�.
	}
}
void UCBComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues(); //���� ź�� ���� ������Ʈ�Ѵ�.
	}
}

void UCBComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance(); //ĳ������ �ִϸ��̼� �ν��Ͻ��� �����´�.
	if (AnimInstance && Character->GetReloadMontage()) {
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}
// ============================================================
// [����ź ������ ����] ThrowGrenadeFinished()
// ------------------------------------------------------------
// ��� ��� : 
//   - ����ź ��ô �� ���� �� ���¸� �����Ѵ�.
// �˰��� ���� : 
//   1. CombatState�� Unoccupied�� ����
//   2. ���⸦ �ٽ� �����տ� ����
//   3. ��ġ ������ ����
// ============================================================
void UCBComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied; //���� ���¸� ����ִ� ���·� �����Ѵ�.
	AttachActorToRightHand(EquippedWeapon); //������ ���⸦ �����տ� �����Ѵ�.
	EquippedWeaponPositionModify(); //������ ������ ��ġ�� �����Ѵ�.
}
// ============================================================
// [����ź �߻�] LaunchGrenade()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���� �÷��̾ ������ ����ź �߻� ��û�� ������.
// �˰��� ���� : 
//   1. ĳ������ ���� ����ź ����
//   2. IsLocallyControlled�� ��� ServerLaunchGrenade() ȣ��
// ============================================================
void UCBComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false); //ĳ������ ������ ����ź�� �����.
	if(Character && Character->IsLocallyControlled()) {
		ServerLaunchGrenade(HitTarget); //������ ����ź �߻� ��û�� ������.
	}
}
// ============================================================
// [���� ����ź �߻� ó��] ServerLaunchGrenade()
// ------------------------------------------------------------
// ��� ��� : 
//   - �������� ���� ����ź(Projectile)�� ���� �� �߻��Ѵ�.
// �˰��� ���� : 
//   1. ���� ����ź ��ġ���� SpawnActor ȣ��
//   2. �浹 ������ ���� ĳ���� ���� ����
// ============================================================
void UCBComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade()) {
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World) {
			AProjectile* Grenade = World->SpawnActor<AProjectile>(GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
			if (Grenade) {
				FCollisionQueryParams QueryParams;
				QueryParams.AddIgnoredActor(SpawnParams.Owner);
				Grenade->CollisionBox->IgnoreActorWhenMoving(SpawnParams.Owner, true);
			}
		}
	}
}
// ============================================================
// [���� ���ε� ó��] ServerReload()
// ------------------------------------------------------------
// ��� ��� : 
//   - CombatState�� Reloading���� ���� �� HandleReload() ȣ��.
// ============================================================
void UCBComponent::ServerReload_Implementation()
{
	CombatState = ECombatState::ECS_Reloading;//���� ���¸� ���ε� ���·� �����Ѵ�
	HandleReload();//���ε带 ó���Ѵ�.
}
// ============================================================
// [���ε� �Ϸ� ó��] FinishReload()
// ------------------------------------------------------------
// ��� ��� : 
//   - ���ε� �ִϸ��̼� ���� �� ȣ��.
// �˰��� ���� : 
//   1. ����: ź�� �� ���� �� ���� ����
//   2. Ŭ���̾�Ʈ: ServerFinishReload() RPC ��û
//   3. �߻� ��ư ���� �� �ڵ� �߻� �����
// ============================================================
void UCBComponent::FinishReload()
{
	if (Character == nullptr) return;
	
	if (Character->HasAuthority()) {
		// �������� ���� ����
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues(); //ź�� ���� ������Ʈ�Ѵ�.
	}
	else {
		// Ŭ���̾�Ʈ������ ���� RPC ȣ��
		ServerFinishReload();
	}
	if (bFireButtonPressed) {
		Fire(); //�߻� ��ư�� �������� �߻��Ѵ�.
	}
}
// ============================================================
// [HUD ��ü ����ȭ] PushAllHUDFromCombat()
// ------------------------------------------------------------
// ��� ��� : 
//   - HUD�� ü��, �ǵ�, ź��, ����ź, ���ھ� �����͸� �� ���� ����.
// ��� �˰��� : 
//   1. Controller�� ������ �� HUD �׸� ���� ����
//   2. ĳ���� �� Health/Shield
//   3. ���� �� Ammo
//   4. PlayerState �� Score/Defeats
// ============================================================
void UCBComponent::PushAllHUDFromCombat()
{
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character ? Character->Controller : nullptr) : Controller;
	if (!Controller) return;

	// ü��/����
	if (Character)
	{
		Controller->SetHUDHealth(Character->GetHealth(), Character->GetMaxHealth());
		Controller->SetHUDShield(Character->GetShield(), Character->GetMaxShield());
	}

	// ��ź ��
	if (EquippedWeapon)
	{
		Controller->SetHUDWeaponAmmo(EquippedWeapon->GetAmmo());
	}
	else
	{
		Controller->SetHUDWeaponAmmo(0);
	}

	// ���� ź��
	UpdateCarriedAmmo(); // ���ο��� Controller->SetHUDCarriedAmmo ȣ����

	// ����ź
	UpdateHUDGrenades();

	// ���ھ�/����
	if (ATFPlayerState* PS = Character ? Character->GetPlayerState<ATFPlayerState>() : nullptr)
	{
		Controller->SetHUDScore(PS->GetScore());
		Controller->SetHUDDefeats(PS->GetDefeats());
	}
}

void UCBComponent::ServerFinishReload_Implementation()
{
	CombatState = ECombatState::ECS_Unoccupied;
	UpdateAmmoValues(); // �����Ǿ� �ִ� ������ ź�� �ݿ�
}
	
void UCBComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //��Ʈ�ѷ��� �����´�.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //��Ʈ�ѷ��� �ִٸ� HUD�� ���� ź���� �����Ѵ�.
	}
	bool bJumpToShotgunEnd = CombatState==ECombatState::ECS_Reloading && EquippedWeapon !=nullptr && EquippedWeapon->GetWeaponType()==EWeaponType::EWT_ShotGun && CarriedAmmo==0; //���� ������ ���� ���θ� �ʱ�ȭ�Ѵ�.
	if(bJumpToShotgunEnd) {
		JumpToShotgunEnd(); //���� ������ �����Ѵ�.
	}
}


void UCBComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingCarriedAmmo); //������ �⺻ ź���� �����Ѵ�.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo); //���� �߻���� �⺻ ź���� �����Ѵ�.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo); //���� �߻���� �⺻ ź���� �����Ѵ�.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo); //���� �߻���� �⺻ ź���� �����Ѵ�.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotgunAmmo); //���� �߻���� �⺻ ź���� �����Ѵ�.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo); //���� �߻���� �⺻ ź���� �����Ѵ�.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeAmmo); //���� �߻���� �⺻ ź���� �����Ѵ�.
}

void UCBComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return; //������ ���Ⱑ ������ �Լ��� �����Ѵ�.

	if (bisAiming) {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), 
			DeltaTime, EquippedWeapon->GetZoomInterpSpeed()); //���� ������ �� FOV�� �����Ѵ�.
	}
	else {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed); //���� ���°� �ƴ� �� FOV�� �����Ѵ�.
	}
	if (Character && Character->GetFollowCamera()) {
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV); //ĳ������ ī�޶� FOV�� ���� FOV�� �����Ѵ�.
	}
}

void UCBComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return; //������ ���Ⱑ ������ �Լ��� �����Ѵ�.
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCBComponent::FireTimerFinished, EquippedWeapon->FireRate); //�߻� Ÿ�̸Ӹ� �����Ѵ�.
}
void UCBComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return; //������ ���Ⱑ ������ �Լ��� �����Ѵ�.
	bCanFire = true; //�߻� ���� ���θ� true�� �����Ѵ�.
	if (bFireButtonPressed && EquippedWeapon->bAutoMatickFire) {
		Fire(); //�߻� ��ư�� �������� �߻��Ѵ�.
	}
	ReloadEmptyWeapon(); //������ ���Ⱑ ��������� ���ε带 �õ��Ѵ�.
}

bool UCBComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false; //������ ���Ⱑ ������ �߻��� �� ����.
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun) return true; //������ ��� ���ε� ���¿����� �߻��� �� �ִ�.
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState==ECombatState::ECS_Unoccupied; //�߻��� �� �ִ� �������� Ȯ���Ѵ�.
}



void UCBComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed; //ĳ������ �ִ� �ȴ� �ӵ��� �⺻ �ӵ��� �����Ѵ�.
		if (Character->GetFollowCamera()) {
			DefaultFOV = Character->GetFollowCamera()->FieldOfView; //�⺻ FOV�� �����Ѵ�.
			CurrentFOV = DefaultFOV; //���� FOV�� �⺻ FOV�� �����Ѵ�.
		}
	}
	if (Character->HasAuthority()) {
		InitializeCarriedAmmo(); //�������� ĳ������ ���� ź���� �ʱ�ȭ�Ѵ�.
	}
}

void UCBComponent::SetAiming(bool bAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return; //ĳ���ͳ� ������ ���Ⱑ ������ �Լ��� �����Ѵ�.
	bisAiming = bAiming; //���� ���θ� �����Ѵ�.
	ServerSetAiming(bAiming); //�������� ���� ���θ� �����Ѵ�.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //���� ���ο� ���� ĳ������ �ִ� �ȴ� �ӵ��� �����Ѵ�.
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) {
		Character->ShowSniperScopeWidget(bAiming); //���� �÷��̾ ���� ���� �� �������� ������ ������ ǥ���Ѵ�.
	}
}


void UCBComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed; //�߻� ��ư�� ���ȴ��� ���θ� �����Ѵ�.
	if (bFireButtonPressed) {
		Fire();
	}
}



void UCBComponent::Fire()
{
	if (CanFire())
	{
		ServerFire(HitTarget); // ������ �ùٸ� Ÿ�� ����
		if (EquippedWeapon) {
			bCanFire = false; //�߻� ���� ���θ� false�� �����Ѵ�.
			CrosshairShootingFactor = 0.85f; //�߻� ��ư�� ������ �� ũ�ν���� ��� ����� ������Ų��.
		}
		StartFireTimer(); //�߻� Ÿ�̸Ӹ� �����Ѵ�.
	}
}

void UCBComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	MulticastFire(TraceHitTargert); //�������� �߻� ��Ƽĳ��Ʈ �Լ��� ȣ���Ѵ�.
}

void UCBComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bisAiming = bAiming; //�������� ���� ���θ� �����Ѵ�.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //���� ���ο� ���� ĳ������ �ִ� �ȴ� �ӵ��� �����Ѵ�.
	}
}

// ============================================================
// [�߻� ��Ƽĳ��Ʈ] MulticastFire()
// ------------------------------------------------------------
// ��� ��� : 
//   - �������� ��� Ŭ���̾�Ʈ���� �߻� �ִϸ��̼ǰ� ȿ�� ����.
//   - ���� ���� ó�� ���� (���ε� �� �߻� ���).
// ============================================================
void UCBComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun) {
		Character->PlayFireMontage(bisAiming); //ĳ������ �߻� ����� ����Ѵ�.
		EquippedWeapon->Fire(TraceHitTargert); //���⸦ �߻��Ѵ�.
		CombatState = ECombatState::ECS_Unoccupied; //�߻� �� ���� ���¸� ����������� �����Ѵ�.
		return; //������ ��� ���ε� ���¿����� �߻��� �� �ִ�.
	}
	if (Character && CombatState==ECombatState::ECS_Unoccupied) {
		Character->PlayFireMontage(bisAiming); //ĳ������ �߻� ����� ����Ѵ�.
		EquippedWeapon->Fire(TraceHitTargert); //���⸦ �߻��Ѵ�.
	}
}
// ============================================================
// [���ؼ� ����] TraceUnderCrosshairs()
// ------------------------------------------------------------
// ��� ��� : 
//   - ȭ�� �߾� ũ�ν���� �������� ����Ʈ���̽� ����.
//   - ���� ����� �浹 ����(FHitResult)�� ��ȯ�Ѵ�.
// �˰��� ���� : 
//   1. ����Ʈ �߾� ��ǥ ���
//   2. DeprojectScreenToWorld()�� ���� ���� ���� ��ȯ
//   3. ����Ʈ���̽�(Visibility ä��) ����
//   4. �浹 �� ImpactPoint ����, ������ ��� ��ǥ�� ����
//   5. ���� ����� Crosshair �������̽� ���� �� ���� ����
// ============================================================
void UCBComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld && Character && Character->GetFollowCamera()) {
		FVector CameraLocation = Character->GetFollowCamera()->GetComponentLocation();
		// ĸ�� ��������ŭ �� �ָ� ���� (�ڱ� �ݸ��� �ۿ��� ����)
		float CapsuleRadius = Character->GetCapsuleComponent() ? Character->GetCapsuleComponent()->GetScaledCapsuleRadius() : 34.f;
		FVector Start = CameraLocation + CrosshairWorldDirection * (CapsuleRadius + 20.f);
		FVector End = Start + (CrosshairWorldDirection * 80000.f);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character);
		if (Character->GetCapsuleComponent())
			QueryParams.AddIgnoredComponent(Character->GetCapsuleComponent());

		TArray<FHitResult> HitResults;
		GetWorld()->LineTraceMultiByChannel(HitResults, Start, End, ECC_Visibility, QueryParams);

		bool bFoundValidHit = false;
		for (const FHitResult& Hit : HitResults) {
			if (Hit.GetActor() && Hit.GetActor() != Character) {
				TraceHitResult = Hit;
				bFoundValidHit = true;
				break;
			}
		}
		// ����� ���� ImpactPoint�� End�� ����
		if (!bFoundValidHit) {
			TraceHitResult = FHitResult();
			TraceHitResult.ImpactPoint = End;
		}

		if (TraceHitResult.GetActor() == Character) {
			UE_LOG(LogTemp, Warning, TEXT("�ڱ� �ڽ� ����"));
		}
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>()) {
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else {
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}
// ============================================================
// [���ؼ� ����] TraceUnderCrosshairs()
// ------------------------------------------------------------
// ��� ��� : 
//   - ȭ�� �߾� ũ�ν���� �������� ����Ʈ���̽� ����.
//   - ���� ����� �浹 ����(FHitResult)�� ��ȯ�Ѵ�.
// �˰��� ���� : 
//   1. ����Ʈ �߾� ��ǥ ���
//   2. DeprojectScreenToWorld()�� ���� ���� ���� ��ȯ
//   3. ����Ʈ���̽�(Visibility ä��) ����
//   4. �浹 �� ImpactPoint ����, ������ ��� ��ǥ�� ����
//   5. ���� ����� Crosshair �������̽� ���� �� ���� ����
// ============================================================
void UCBComponent::SetHUDCrossharis(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->GetController()) : Controller; // �÷��̾� ��Ʈ�ѷ��� �����´�.
	if (Controller) {
		TFHUD = TFHUD == nullptr ? Cast<ATFHUD>(Controller->GetHUD()) : TFHUD; // HUD�� �����´�.
		if (TFHUD) {

			if (EquippedWeapon) {
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter; // ���Ⱑ ������ �߾� ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft; // ���Ⱑ ������ ���� ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight; // ���Ⱑ ������ ������ ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop; // ���Ⱑ ������ ���� ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom; // ���Ⱑ ������ �Ʒ��� ũ�ν��� �����Ѵ�.
			}
			else {
				HUDPackage.CrosshairsCenter = nullptr; // ���Ⱑ ������ �߾� ũ�ν��� �����Ѵ�.
				HUDPackage.CrosshairsLeft = nullptr; // ���Ⱑ ������ ���� ũ�ν��� �����Ѵ�.
					HUDPackage.CrosshairsRight = nullptr; // ���Ⱑ ������ ������ ũ�ν��� �����Ѵ�.
					HUDPackage.CrosshairsTop = nullptr; // ���Ⱑ ������ ���� ũ�ν��� �����Ѵ�.
					HUDPackage.CrosshairsBottom = nullptr; // ���Ⱑ ������ �Ʒ��� ũ�ν��� �����Ѵ�.
			}
			//ũ�ν���� �������� ���
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed); // �ȴ� �ӵ� ������ �����Ѵ�.
			FVector2D VelocityMultiplierRange(0.f, 1.f); // �ӵ� ���� ������ �����Ѵ�.
			FVector Velocity = Character->GetVelocity(); // ĳ������ �ӵ��� �����´�.
			Velocity.Z = 0.f; // Z���� �ӵ��� 0���� �����.

			CrosshairVelocityFactor=FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange,
				Velocity.Size()); // �ȴ� �ӵ��� ���� ũ�ν���� �������带 ����Ѵ�.
			if (bisAiming) {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, AimFactorValue1, DeltaTime, AimFactorValue2); // ���� ������ �� ũ�ν���� �������带 �����Ѵ�.
			}
			else {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, AimFactorValue2); // ���� ���°� �ƴ� �� ũ�ν���� �������带 �����Ѵ�.
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 10.f); // ��� ������ �� ũ�ν���� �������带 �����Ѵ�.
			HUDPackage.CrosshairSpread = 0.5f+ CrosshairVelocityFactor - CrosshairAimFactor+CrosshairShootingFactor; // ũ�ν���� �������带 �����Ѵ�.
			TFHUD->SetHUDPackage(HUDPackage); // HUD ��Ű���� �����Ѵ�.
		}
	}
}
// ============================================================
// [����ź ��ô] ThrowGrenade()
// ------------------------------------------------------------
// ��� ��� : 
//   - ����ź ��ô ���·� ��ȯ�ϰ� �ִϸ��̼� �� HUD ����.
// �˰��� ���� : 
//   1. ���ε�/���� ���� Ȯ��
//   2. CombatState�� ThrowingGrenade�� ����
//   3. ����ź �ִϸ��̼� ��� �� �޼� ����
//   4. ���� ���� ���ο� ���� RPC ó��
//   5. HUD�� ����ź ���� ����
// ============================================================
void UCBComponent::ThrowGrenade()
{
	if (CombatState == ECombatState::ECS_Reloading) return; //���ε� ���¿����� ����ź�� ������ �ʴ´�.
	if (Grenades == 0) return; //����ź�� ������ �Լ��� �����Ѵ�.
	if (EquippedWeapon == nullptr || Character == nullptr) return; //������ ���⳪ ĳ���Ͱ� ������ �Լ��� �����Ѵ�.
	if (CombatState != ECombatState::ECS_Unoccupied) return; //���� ���°� ������� ������ ����ź�� ������ �ʴ´�.
	CombatState = ECombatState::ECS_ThrowingGrenade;//���� ���¸� ����ź ������� �����Ѵ�.
	if (Character) {
		Character->PlayThrowGrendadeMontage(); //ĳ������ ����ź ������ ��Ÿ�ָ� ����Ѵ�.
		AttachActorToLeftHand(EquippedWeapon); //���⸦ �޼տ� �����Ѵ�.
		ShowAttachedGrenade(true); //ĳ������ ������ ����ź�� ���̰� �Ѵ�.
	}
	if (Character && !Character->HasAuthority()) {
		ServerThrowGrenade(); //������ ����ź ������ ��û�� ������.
	}
	if(Character && Character->HasAuthority()) {
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades); //����ź ������ ���ҽ�Ų��.
		UpdateHUDGrenades(); //HUD�� ����ź ������ ������Ʈ�Ѵ�.
	}
}

void UCBComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return; //����ź�� ������ �Լ��� �����Ѵ�.
	CombatState = ECombatState::ECS_ThrowingGrenade;//���� ���¸� ����ź ������� �����Ѵ�.
	if (Character) {
		Character->PlayThrowGrendadeMontage(); //ĳ������ ����ź ������ ��Ÿ�ָ� ����Ѵ�.
		AttachActorToLeftHand(EquippedWeapon); //���⸦ �޼տ� �����Ѵ�.
		ShowAttachedGrenade(true); //ĳ������ ������ ����ź�� ���̰� �Ѵ�.
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades); //����ź ������ ���ҽ�Ų��.
	UpdateHUDGrenades(); //HUD�� ����ź ������ ������Ʈ�Ѵ�.
}
void UCBComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //��Ʈ�ѷ��� �����´�.
	if (Controller) {
		Controller->SetHUDGrenadeCount(Grenades); //��Ʈ�ѷ��� �ִٸ� HUD�� ����ź ������ �����Ѵ�.
	}
}
void UCBComponent::OnRep_Grenades()
{
	UpdateHUDGrenades(); //HUD�� ����ź ������ ������Ʈ�Ѵ�.
}
void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled()) // ���� �÷��̾� ��Ʈ�ѷ��� ��쿡�� �����Ѵ�.
	{
		FHitResult HitResult; // Ʈ���̽� ����� ������ ����
		TraceUnderCrosshairs(HitResult); // �� �����Ӹ��� ȭ�� �߾� �Ʒ��� ��ü�� �����Ѵ�.
		HitTarget = HitResult.ImpactPoint; // ��Ʈ Ÿ���� Ʈ���̽� ����� �浹 �������� �����Ѵ�.
		SetHUDCrossharis(DeltaTime); // �� �����Ӹ��� HUD�� ũ�ν��� �����Ѵ�.
		InterpFOV(DeltaTime); // FOV�� �����Ѵ�.
	}
	if (EquippedWeapon) {
		UpdateHUDGrenades(); //HUD�� ����ź ������ ������Ʈ�Ѵ�.
	}
}
bool UCBComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr && !bisAiming && CombatState!=ECombatState::ECS_ThrowingGrenade && 
		CombatState!=ECombatState::ECS_Reloading); //������ ����� ���� ���Ⱑ ��� �����ϴ��� Ȯ���Ѵ�.
}
void UCBComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType)) {
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo); //�ش� ���� Ÿ���� ���� ź���� ������Ų��.
		UpdateCarriedAmmo(); //���� ź���� ������Ʈ�Ѵ�.
	}
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType) {
		Reload(); //������ ���Ⱑ ����ְ�, �ش� ���� Ÿ�԰� ������ ���ε带 �õ��Ѵ�.
	}
}

