

#include "CBComponent.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
UCBComponent::UCBComponent()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UCBComponent::EquipWeapon(AWeapon* WeaponEquip)
{
	if (Character == nullptr || WeaponEquip==nullptr) return; //ĳ���Ͱ� ������ ����
	EquippedWeapon = WeaponEquip; //������ ���⸦ �����Ѵ�.
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); //������ ���� ���¸� �����Ѵ�.
	const USkeletalMeshSocket* HandSocket=Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));//ĳ������ ������ ������ �����´�.
	if (HandSocket) //������ �����ϸ�
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh()); //���Ͽ� ���⸦ �����Ѵ�.
	}
	EquippedWeapon->SetOwner(Character); //������ �����ڸ� ĳ���ͷ� �����Ѵ�.
	EquippedWeapon->ShowPickupWidget(false); //���� ������ �����.
	EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision); //���� ���Ǿ��� �浹�� ��Ȱ��ȭ�Ѵ�.
}


void UCBComponent::BeginPlay()
{
	Super::BeginPlay();


}


void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

