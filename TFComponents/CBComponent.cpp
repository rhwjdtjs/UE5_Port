

#include "CBComponent.h"
#include "UnrealProject_7A/Weapon/Weapon.h"
#include "UnrealProject_7A/Character/TimeFractureCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" //복제를 위한 헤더파일을 포함시킨다.
#include "GameFramework/CharacterMovementComponent.h" //캐릭터 이동 컴포넌트를 포함시킨다.
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
// [무기 장착] EquipWeapon()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 캐릭터가 새로운 무기를 장착한다.
//   - 주 무기 / 보조 무기를 구분하여 올바른 슬롯에 장착.
// 알고리즘 설명 : 
//   1. 캐릭터 또는 무기가 없으면 리턴
//   2. 현재 전투 상태가 비어있지 않으면 장착 불가
//   3. 장착된 무기가 있다면 보조무기 슬롯으로 이동
//   4. 없다면 주무기로 장착
//   5. 캐릭터의 Yaw 회전 제어 활성화
// ============================================================
void UCBComponent::EquipWeapon(AWeapon* WeaponEquip)
{
	if (Character == nullptr || WeaponEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied)return; //전투 상태가 비어있지 않으면 장착하지 않는다.
	if(EquippedWeapon !=nullptr && SecondaryWeapon==nullptr) //장착된 무기가 있고 보조 무기가 없으면
	{
		EquipSecondaryWeapon(WeaponEquip); //보조 무기를 장착한다.
	}
	else
	{
		EquipPrimaryWeapon(WeaponEquip); //주 무기를 장착한다.
	}

	Character->bUseControllerRotationYaw = true; //캐릭터가 컨트롤러의 Yaw 회전을 사용하도록 설정한다.
	
}
// ============================================================
// [무기 교체] SwapWeapons()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 주무기와 보조무기의 슬롯을 서로 교체한다.
//   - HUD 탄약 정보 갱신 및 사운드 재생 포함.
// 알고리즘 설명 : 
//   1. 리로드 중이면 교체 불가
//   2. EquippedWeapon, SecondaryWeapon을 서로 교환
//   3. 주무기는 오른손으로, 보조무기는 등으로 이동
//   4. HUD 및 탄약 수, 사운드 업데이트
// ============================================================
void UCBComponent::SwapWeapons()
{
	if (CombatState == ECombatState::ECS_Reloading) return;
	AWeapon* TempWeapon = EquippedWeapon; //장착된 무기를 임시 변수에 저장한다.
	EquippedWeapon = SecondaryWeapon; //장착된 무기에 보조 무기를 설정한다.
	SecondaryWeapon = TempWeapon; //보조 무기에 임시 변수에 저장된 장착된 무기를 설정한다.

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeaponPositionModify();
	EquippedWeapon->SetHUDAmmo(); //장착된 무기의 HUD 탄약을 설정한다.
	UpdateCarriedAmmo(); //보유 탄약을 업데이트한다.
	PlayEquipSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBack(SecondaryWeapon); //장착된 무기를 등에 부착한다.
}
// ============================================================
// [주무기 장착] EquipPrimaryWeapon()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 새로운 무기를 주무기 슬롯에 장착한다.
// 알고리즘 설명 : 
//   1. 기존 무기가 있다면 Drop 처리
//   2. 무기 상태를 “Equipped”로 변경
//   3. 오른손 소켓에 부착
//   4. 소유자 및 HUD 갱신
//   5. 장착 사운드 재생 및 무기 충돌 비활성화
// ============================================================
void UCBComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon(); //이미 장착된 무기가 있으면 드롭한다.
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeaponPositionModify();
	EquippedWeapon->SetHUDAmmo(); //장착된 무기의 HUD 탄약을 설정한다.
	UpdateCarriedAmmo(); //보유 탄약을 업데이트한다.
	PlayEquipSound(WeaponToEquip);
	EquippedWeapon->ShowPickupWidget(false);
	EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ReloadEmptyWeapon();
}
// ============================================================
// [보조무기 장착] EquipSecondaryWeapon()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 무기를 보조무기 슬롯에 장착한다.
// 알고리즘 설명 : 
//   1. 무기 상태를 “EquippedSecondary”로 설정
//   2. 등(back socket)에 부착
//   3. 사운드 재생 및 소유자 설정
// ============================================================
void UCBComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBack(WeaponToEquip); //장착된 무기를 등에 부착한다.
	PlayEquipSound(WeaponToEquip);
	SecondaryWeapon->SetOwner(Character);
}
// ============================================================
// [장착 무기 복제 응답] OnRep_EquippedWeapon()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 클라이언트에서 장착 무기 정보가 복제되었을 때,
//     시각적/사운드/탄약 데이터를 반영한다.
// 사용 기술 : 
//   - 언리얼 네트워크 복제 (RepNotify)
//   - AttachToComponent() / HUD 연동
// ============================================================
void UCBComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon) {
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		EquippedWeapon->EnableCustomDepth(false); //장착된 무기의 커스텀 깊이를 비활성화한다.
		EquippedWeapon->SetHUDAmmo(); //장착된 무기의 HUD 탄약을 설정한다.
	}
	PlayEquipSound(EquippedWeapon);
}
// ============================================================
// [보조무기 복제 응답] OnRep_SecondaryWeapon()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 클라이언트에서 보조무기 정보가 갱신될 때 호출된다.
//   - 무기를 등(back socket)에 부착하고 장착 사운드 재생.
// ============================================================
void UCBComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character) {
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBack(SecondaryWeapon); //장착된 무기를 등에 부착한다.
		PlayEquipSound(EquippedWeapon);

	}
}
// ============================================================
// [무기 드롭] DropEquippedWeapon()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 현재 장착 중인 무기를 바닥에 떨어뜨린다.
//   - 무기 드롭은 물리/충돌이 다시 활성화된다.
// ============================================================
void UCBComponent::DropEquippedWeapon()
{
	if (EquippedWeapon) {
		EquippedWeapon->DropWeapon(); //이미 장착된 무기가 있으면 드롭한다.
	}
}
// ============================================================
// [무기 오른손 부착] AttachActorToRightHand()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 무기를 캐릭터의 오른손 소켓("RightHandSocket")에 부착한다.
// 알고리즘 설명 : 
//   1. 캐릭터, 메시, 액터 유효성 검사
//   2. FAttachmentTransformRules::SnapToTargetNotIncludingScale 사용
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
// [무기 왼손 부착] AttachActorToLeftHand()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 무기를 캐릭터의 왼손 소켓("LeftHandSocket")에 부착한다.
//   - 리로드나 수류탄 투척 시 임시 위치 변경용으로 사용.
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
// [무기 등 부착] AttachActorToBack()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 무기를 캐릭터의 등 소켓("BackWeaponSocket")에 부착한다.
//   - 스나이퍼 소총의 경우 회전/위치 오프셋 추가 적용.
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
			FRotator Offset = FRotator(0.f, 90.f, 0.f); // Z축을 위로 10만큼 올리기
			ActorToAttach->GetRootComponent()->SetRelativeRotation(Offset); //무기의 루트 컴포넌트 위치를 조정한다.
			ActorToAttach->GetRootComponent()->SetRelativeLocation(FOffset); //무기의 루트 컴포넌트 위치를 조정한다.
		}
	}
}
// ============================================================
// [보유 탄약 갱신] UpdateCarriedAmmo()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 장착된 무기의 타입에 따라 보유 탄약 수를 HUD에 반영한다.
// 알고리즘 설명 : 
//   1. 장착된 무기 확인 후 CarriedAmmoMap에서 해당 타입 검색
//   2. 현재 탄약 수를 Controller HUD에 전달
// ============================================================
void UCBComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr)return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) {
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; //장착된 무기의 보유 탄약을 설정한다.
	}
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //컨트롤러를 가져온다.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //컨트롤러가 있다면 HUD에 보유 탄약을 설정한다.
	}
}
void UCBComponent::PlayEquipSound(AWeapon* WeaponToEquip)
{
	if (Character&& WeaponToEquip && WeaponToEquip->EquipSound) {
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSound, Character->GetActorLocation()); //장착 사운드를 재생한다.
	}
}
void UCBComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon&&EquippedWeapon->IsEmpty()) {
		Reload(); //장착된 무기가 비어있으면 리로드를 시도한다.
	}
}
// ============================================================
// [무기 위치 수정] EquippedWeaponPositionModify()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 무기 종류(EWeaponType)에 따라 손에 들었을 때의 위치/회전 보정.
// 알고리즘 설명 : 
//   - 샷건: 위치를 약간 위로
//   - 권총: 약간 아래로
//   - 스나이퍼 라이플: 회전 + 위치 오프셋 적용
// ============================================================
void UCBComponent::EquippedWeaponPositionModify()
{
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun)
	{
		FVector Offset = FVector(0.f, 0.f, 23.f); // Z축을 위로 10만큼 올리기
		EquippedWeapon->GetRootComponent()->SetRelativeLocation(Offset); //무기의 루트 컴포넌트 위치를 조정한다.
	}
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol) {
		FVector Offset = FVector(0.f, 0.f, -5.f); // Z축을 위로 10만큼 올리기
		EquippedWeapon->GetRootComponent()->SetRelativeLocation(Offset); //무기의 루트 컴포넌트 위치를 조정한다.
	}
	if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) {
		FVector FOffset = FVector(0.f, 40.f, 22.f);
		FRotator Offset = FRotator(0.f, 90.f, 0.f); // Z축을 위로 10만큼 올리기
		EquippedWeapon->GetRootComponent()->SetRelativeRotation(Offset); //무기의 루트 컴포넌트 위치를 조정한다.
		EquippedWeapon->GetRootComponent()->SetRelativeLocation(FOffset); //무기의 루트 컴포넌트 위치를 조정한다.
	}
}
// ============================================================
// [네트워크 복제 등록] GetLifetimeReplicatedProps()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 언리얼의 DOREPLIFETIME 매크로로 네트워크 복제 변수를 지정한다.
// 복제 항목 : 
//   - EquippedWeapon, SecondaryWeapon, bisAiming, CombatState, Grenades
//   - CarriedAmmo (소유자 전용 복제)
// ============================================================
void UCBComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCBComponent, EquippedWeapon); //장착된 무기를 복제한다.
	DOREPLIFETIME(UCBComponent, SecondaryWeapon); //보조 무기를 복제한다.
	DOREPLIFETIME(UCBComponent, bisAiming);// 조준 여부를 복제한다.
	DOREPLIFETIME(UCBComponent, CombatState); //전투 상태를 복제한다.
	DOREPLIFETIME(UCBComponent, Grenades); //수류탄 개수를 복제한다.
	DOREPLIFETIME_CONDITION(UCBComponent, CarriedAmmo, COND_OwnerOnly); // 소유자만 탄약을 복제한다.
	//DOREPLIFETIME_CONDITION(UCBComponent, EquippedWeapon, COND_OwnerOnly); //장착된 무기를 복제하는데, 조건은 소유자만 복제한다는 뜻이다.
}
// ============================================================
// [전투 상태 복제 응답] OnRep_CombatState()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 서버에서 전투 상태가 변경되면 클라이언트 측에서 상태를 동기화한다.
// 알고리즘 설명 : 
//   - Reloading → 리로드 처리
//   - ThrowingGrenade → 수류탄 투척 모션 및 왼손 무기 이동
//   - Unoccupied → 발사 버튼 유지 시 자동 발사
// ============================================================
void UCBComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed) {
			Fire(); //발사 버튼이 눌렸으면 발사한다.
		}
		break;
	case ECombatState::ECS_Reloading:
		UE_LOG(LogTemp, Warning, TEXT("Client: OnRep_CombatState - Reloading started"));
			HandleReload(); //리로드를 처리한다.
		
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled()) {
			Character->PlayThrowGrendadeMontage(); //캐릭터의 수류탄 던지기 몽타주를 재생한다.
			AttachActorToLeftHand(EquippedWeapon); //장착된 무기를 왼손에 부착한다.
			ShowAttachedGrenade(true); //캐릭터의 부착된 수류탄을 보이게 한다.
		}
	case ECombatState::ECS_MAX:
		break;
	}
}

void UCBComponent::Reload() {
	if (CarriedAmmo > 0 && CombatState !=ECombatState::ECS_Reloading && EquippedWeapon && !EquippedWeapon->IsFull()) {
		ServerReload(); //서버에 리로드 요청을 보낸다.
	}
}

void UCBComponent::HandleReload()
{
	if (Character) {
		Character->PlayReloadMontage(); //캐릭터의 리로드 몽타주를 재생한다.
	}
}
// ============================================================
// [리로드 양 계산] AmountToReload()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 탄창 여유 공간(RoomInMag)과 보유 탄약 중 최소값 계산.
//   - 리로드 가능한 실제 탄약 수 반환.
// ============================================================
int32 UCBComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0; //장착된 무기가 없으면 0을 반환한다.
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo(); //탄창에 남은 공간을 계산한다.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) { // 장착된 무기의 타입이 보유 탄약 맵에 있는지 확인
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; // 장착된 무기 타입의 현재 보유 탄약 수를 가져옴
		int32 Least = FMath::Min(RoomInMag, AmountCarried); // 탄창에 남은 공간과 보유 탄약 중 더 작은 값을 구함
		return FMath::Clamp(RoomInMag, 0, Least); // 0과 Least 사이의 값으로 RoomInMag를 제한하여 반환 (최대 장전 가능한 탄약 수)
	}
	return 0;
}
// ============================================================
// [탄약 수 업데이트] UpdateAmmoValues()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 리로드 시 보유 탄약을 차감하고, 탄창에 추가한다.
//   - HUD 탄약 표시를 갱신한다.
// ============================================================
void UCBComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return; //캐릭터가 없으면 함수를 종료한다.
	int32 ReloadAmount = AmountToReload(); //리로드할 양을 계산한다.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) { // 장착된 무기의 타입이 보유 탄약 맵에 있는지 확인
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount; // 보유 탄약에서 리로드할 양을 차감한다.
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; //현재 보유 탄약을 업데이트한다.
	}
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //컨트롤러를 가져온다.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //컨트롤러가 있다면 HUD에 보유 탄약을 설정한다.
	}
	EquippedWeapon->AddAmmo(-ReloadAmount); //장착된 무기에 리로드할 양을 추가한다.
}
void UCBComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return; //캐릭터가 없으면 함수를 종료한다.
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType())) { // 장착된 무기의 타입이 보유 탄약 맵에 있는지 확인
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1; // 보유 탄약에서 리로드할 양을 차감한다.
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; //현재 보유 탄약을 업데이트한다.
	}
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //컨트롤러를 가져온다.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //컨트롤러가 있다면 HUD에 보유 탄약을 설정한다.
	}
	EquippedWeapon->AddAmmo(-1); //장착된 무기에 리로드할 양을 추가한다.
	bCanFire = true; //발사 가능 여부를 true로 설정한다.
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0) {
		JumpToShotgunEnd(); //장착된 무기가 가득 차면 샷건 끝으로 점프한다.
	}
}
void UCBComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade()) {
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade); //캐릭터의 부착된 수류탄의 가시성을 설정한다.
	}
}
void UCBComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues(); //샷건 탄약 값을 업데이트한다.
	}
}

void UCBComponent::JumpToShotgunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance(); //캐릭터의 애니메이션 인스턴스를 가져온다.
	if (AnimInstance && Character->GetReloadMontage()) {
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}
// ============================================================
// [수류탄 던지기 종료] ThrowGrenadeFinished()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 수류탄 투척 후 무기 및 상태를 복구한다.
// 알고리즘 설명 : 
//   1. CombatState를 Unoccupied로 복귀
//   2. 무기를 다시 오른손에 부착
//   3. 위치 오프셋 수정
// ============================================================
void UCBComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied; //전투 상태를 비어있는 상태로 설정한다.
	AttachActorToRightHand(EquippedWeapon); //장착된 무기를 오른손에 부착한다.
	EquippedWeaponPositionModify(); //장착된 무기의 위치를 수정한다.
}
// ============================================================
// [수류탄 발사] LaunchGrenade()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 로컬 플레이어가 서버로 수류탄 발사 요청을 보낸다.
// 알고리즘 설명 : 
//   1. 캐릭터의 부착 수류탄 숨김
//   2. IsLocallyControlled일 경우 ServerLaunchGrenade() 호출
// ============================================================
void UCBComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false); //캐릭터의 부착된 수류탄을 숨긴다.
	if(Character && Character->IsLocallyControlled()) {
		ServerLaunchGrenade(HitTarget); //서버에 수류탄 발사 요청을 보낸다.
	}
}
// ============================================================
// [서버 수류탄 발사 처리] ServerLaunchGrenade()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 서버에서 실제 수류탄(Projectile)을 생성 및 발사한다.
// 알고리즘 설명 : 
//   1. 부착 수류탄 위치에서 SpawnActor 호출
//   2. 충돌 방지를 위해 캐릭터 무시 설정
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
// [서버 리로드 처리] ServerReload()
// ------------------------------------------------------------
// 기능 요약 : 
//   - CombatState를 Reloading으로 변경 후 HandleReload() 호출.
// ============================================================
void UCBComponent::ServerReload_Implementation()
{
	CombatState = ECombatState::ECS_Reloading;//전투 상태를 리로드 상태로 설정한다
	HandleReload();//리로드를 처리한다.
}
// ============================================================
// [리로드 완료 처리] FinishReload()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 리로드 애니메이션 종료 시 호출.
// 알고리즘 설명 : 
//   1. 서버: 탄약 수 갱신 및 상태 복귀
//   2. 클라이언트: ServerFinishReload() RPC 요청
//   3. 발사 버튼 유지 시 자동 발사 재시작
// ============================================================
void UCBComponent::FinishReload()
{
	if (Character == nullptr) return;
	
	if (Character->HasAuthority()) {
		// 서버에서 직접 실행
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues(); //탄약 값을 업데이트한다.
	}
	else {
		// 클라이언트에서는 서버 RPC 호출
		ServerFinishReload();
	}
	if (bFireButtonPressed) {
		Fire(); //발사 버튼이 눌렸으면 발사한다.
	}
}
// ============================================================
// [HUD 전체 동기화] PushAllHUDFromCombat()
// ------------------------------------------------------------
// 기능 요약 : 
//   - HUD의 체력, 실드, 탄약, 수류탄, 스코어 데이터를 한 번에 갱신.
// 사용 알고리즘 : 
//   1. Controller를 가져와 각 HUD 항목 직접 갱신
//   2. 캐릭터 → Health/Shield
//   3. 무기 → Ammo
//   4. PlayerState → Score/Defeats
// ============================================================
void UCBComponent::PushAllHUDFromCombat()
{
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character ? Character->Controller : nullptr) : Controller;
	if (!Controller) return;

	// 체력/쉴드
	if (Character)
	{
		Controller->SetHUDHealth(Character->GetHealth(), Character->GetMaxHealth());
		Controller->SetHUDShield(Character->GetShield(), Character->GetMaxShield());
	}

	// 장탄 수
	if (EquippedWeapon)
	{
		Controller->SetHUDWeaponAmmo(EquippedWeapon->GetAmmo());
	}
	else
	{
		Controller->SetHUDWeaponAmmo(0);
	}

	// 보유 탄약
	UpdateCarriedAmmo(); // 내부에서 Controller->SetHUDCarriedAmmo 호출함

	// 수류탄
	UpdateHUDGrenades();

	// 스코어/데스
	if (ATFPlayerState* PS = Character ? Character->GetPlayerState<ATFPlayerState>() : nullptr)
	{
		Controller->SetHUDScore(PS->GetScore());
		Controller->SetHUDDefeats(PS->GetDefeats());
	}
}

void UCBComponent::ServerFinishReload_Implementation()
{
	CombatState = ECombatState::ECS_Unoccupied;
	UpdateAmmoValues(); // 누락되어 있던 서버측 탄약 반영
}
	
void UCBComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //컨트롤러를 가져온다.
	if (Controller) {
		Controller->SetHUDCarriedAmmo(CarriedAmmo); //컨트롤러가 있다면 HUD에 보유 탄약을 설정한다.
	}
	bool bJumpToShotgunEnd = CombatState==ECombatState::ECS_Reloading && EquippedWeapon !=nullptr && EquippedWeapon->GetWeaponType()==EWeaponType::EWT_ShotGun && CarriedAmmo==0; //샷건 끝으로 점프 여부를 초기화한다.
	if(bJumpToShotgunEnd) {
		JumpToShotgunEnd(); //샷건 끝으로 점프한다.
	}
}


void UCBComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingCarriedAmmo); //소총의 기본 탄약을 설정한다.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo); //로켓 발사기의 기본 탄약을 설정한다.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo); //로켓 발사기의 기본 탄약을 설정한다.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo); //로켓 발사기의 기본 탄약을 설정한다.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotgunAmmo); //로켓 발사기의 기본 탄약을 설정한다.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo); //로켓 발사기의 기본 탄약을 설정한다.
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeAmmo); //로켓 발사기의 기본 탄약을 설정한다.
}

void UCBComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return; //장착된 무기가 없으면 함수를 종료한다.

	if (bisAiming) {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), 
			DeltaTime, EquippedWeapon->GetZoomInterpSpeed()); //조준 상태일 때 FOV를 보간한다.
	}
	else {
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed); //조준 상태가 아닐 때 FOV를 보간한다.
	}
	if (Character && Character->GetFollowCamera()) {
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV); //캐릭터의 카메라 FOV를 현재 FOV로 설정한다.
	}
}

void UCBComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return; //장착된 무기가 없으면 함수를 종료한다.
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCBComponent::FireTimerFinished, EquippedWeapon->FireRate); //발사 타이머를 시작한다.
}
void UCBComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return; //장착된 무기가 없으면 함수를 종료한다.
	bCanFire = true; //발사 가능 여부를 true로 설정한다.
	if (bFireButtonPressed && EquippedWeapon->bAutoMatickFire) {
		Fire(); //발사 버튼이 눌렸으면 발사한다.
	}
	ReloadEmptyWeapon(); //장착된 무기가 비어있으면 리로드를 시도한다.
}

bool UCBComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false; //장착된 무기가 없으면 발사할 수 없다.
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun) return true; //샷건의 경우 리로드 상태에서도 발사할 수 있다.
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState==ECombatState::ECS_Unoccupied; //발사할 수 있는 상태인지 확인한다.
}



void UCBComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = baseWalkSpeed; //캐릭터의 최대 걷는 속도를 기본 속도로 설정한다.
		if (Character->GetFollowCamera()) {
			DefaultFOV = Character->GetFollowCamera()->FieldOfView; //기본 FOV를 설정한다.
			CurrentFOV = DefaultFOV; //현재 FOV를 기본 FOV로 설정한다.
		}
	}
	if (Character->HasAuthority()) {
		InitializeCarriedAmmo(); //서버에서 캐릭터의 보유 탄약을 초기화한다.
	}
}

void UCBComponent::SetAiming(bool bAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return; //캐릭터나 장착된 무기가 없으면 함수를 종료한다.
	bisAiming = bAiming; //조준 여부를 설정한다.
	ServerSetAiming(bAiming); //서버에서 조준 여부를 설정한다.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //조준 여부에 따라 캐릭터의 최대 걷는 속도를 설정한다.
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle) {
		Character->ShowSniperScopeWidget(bAiming); //로컬 플레이어가 조준 중일 때 스나이퍼 스코프 위젯을 표시한다.
	}
}


void UCBComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed; //발사 버튼이 눌렸는지 여부를 설정한다.
	if (bFireButtonPressed) {
		Fire();
	}
}



void UCBComponent::Fire()
{
	if (CanFire())
	{
		ServerFire(HitTarget); // 서버에 올바른 타겟 전달
		if (EquippedWeapon) {
			bCanFire = false; //발사 가능 여부를 false로 설정한다.
			CrosshairShootingFactor = 0.85f; //발사 버튼이 눌렸을 때 크로스헤어 사격 계수를 증가시킨다.
		}
		StartFireTimer(); //발사 타이머를 시작한다.
	}
}

void UCBComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	MulticastFire(TraceHitTargert); //서버에서 발사 멀티캐스트 함수를 호출한다.
}

void UCBComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bisAiming = bAiming; //서버에서 조준 여부를 설정한다.
	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimingWalkSpeed : baseWalkSpeed; //조준 여부에 따라 캐릭터의 최대 걷는 속도를 설정한다.
	}
}

// ============================================================
// [발사 멀티캐스트] MulticastFire()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 서버에서 모든 클라이언트에게 발사 애니메이션과 효과 전파.
//   - 샷건 예외 처리 포함 (리로드 중 발사 허용).
// ============================================================
void UCBComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTargert)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun) {
		Character->PlayFireMontage(bisAiming); //캐릭터의 발사 모션을 재생한다.
		EquippedWeapon->Fire(TraceHitTargert); //무기를 발사한다.
		CombatState = ECombatState::ECS_Unoccupied; //발사 후 전투 상태를 비어있음으로 설정한다.
		return; //샷건의 경우 리로드 상태에서도 발사할 수 있다.
	}
	if (Character && CombatState==ECombatState::ECS_Unoccupied) {
		Character->PlayFireMontage(bisAiming); //캐릭터의 발사 모션을 재생한다.
		EquippedWeapon->Fire(TraceHitTargert); //무기를 발사한다.
	}
}
// ============================================================
// [조준선 추적] TraceUnderCrosshairs()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 화면 중앙 크로스헤어 기준으로 라인트레이스 수행.
//   - 조준 대상의 충돌 정보(FHitResult)를 반환한다.
// 알고리즘 설명 : 
//   1. 뷰포트 중앙 좌표 계산
//   2. DeprojectScreenToWorld()로 월드 방향 벡터 변환
//   3. 라인트레이스(Visibility 채널) 수행
//   4. 충돌 시 ImpactPoint 저장, 없으면 허공 좌표로 설정
//   5. 조준 대상이 Crosshair 인터페이스 구현 시 색상 변경
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
		// 캡슐 반지름만큼 더 멀리 시작 (자기 콜리전 밖에서 시작)
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
		// 허공일 때는 ImpactPoint를 End로 설정
		if (!bFoundValidHit) {
			TraceHitResult = FHitResult();
			TraceHitResult.ImpactPoint = End;
		}

		if (TraceHitResult.GetActor() == Character) {
			UE_LOG(LogTemp, Warning, TEXT("자기 자신 맞음"));
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
// [조준선 추적] TraceUnderCrosshairs()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 화면 중앙 크로스헤어 기준으로 라인트레이스 수행.
//   - 조준 대상의 충돌 정보(FHitResult)를 반환한다.
// 알고리즘 설명 : 
//   1. 뷰포트 중앙 좌표 계산
//   2. DeprojectScreenToWorld()로 월드 방향 벡터 변환
//   3. 라인트레이스(Visibility 채널) 수행
//   4. 충돌 시 ImpactPoint 저장, 없으면 허공 좌표로 설정
//   5. 조준 대상이 Crosshair 인터페이스 구현 시 색상 변경
// ============================================================
void UCBComponent::SetHUDCrossharis(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->GetController()) : Controller; // 플레이어 컨트롤러를 가져온다.
	if (Controller) {
		TFHUD = TFHUD == nullptr ? Cast<ATFHUD>(Controller->GetHUD()) : TFHUD; // HUD를 가져온다.
		if (TFHUD) {

			if (EquippedWeapon) {
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter; // 무기가 있으면 중앙 크로스헤어를 설정한다.
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft; // 무기가 있으면 왼쪽 크로스헤어를 설정한다.
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight; // 무기가 있으면 오른쪽 크로스헤어를 설정한다.
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop; // 무기가 있으면 위쪽 크로스헤어를 설정한다.
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom; // 무기가 있으면 아래쪽 크로스헤어를 설정한다.
			}
			else {
				HUDPackage.CrosshairsCenter = nullptr; // 무기가 있으면 중앙 크로스헤어를 설정한다.
				HUDPackage.CrosshairsLeft = nullptr; // 무기가 있으면 왼쪽 크로스헤어를 설정한다.
					HUDPackage.CrosshairsRight = nullptr; // 무기가 있으면 오른쪽 크로스헤어를 설정한다.
					HUDPackage.CrosshairsTop = nullptr; // 무기가 있으면 위쪽 크로스헤어를 설정한다.
					HUDPackage.CrosshairsBottom = nullptr; // 무기가 있으면 아래쪽 크로스헤어를 설정한다.
			}
			//크로스헤어 스프레드 계산
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed); // 걷는 속도 범위를 설정한다.
			FVector2D VelocityMultiplierRange(0.f, 1.f); // 속도 곱셈 범위를 설정한다.
			FVector Velocity = Character->GetVelocity(); // 캐릭터의 속도를 가져온다.
			Velocity.Z = 0.f; // Z축의 속도를 0으로 만든다.

			CrosshairVelocityFactor=FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange,
				Velocity.Size()); // 걷는 속도에 따라 크로스헤어 스프레드를 계산한다.
			if (bisAiming) {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, AimFactorValue1, DeltaTime, AimFactorValue2); // 조준 상태일 때 크로스헤어 스프레드를 보간한다.
			}
			else {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, AimFactorValue2); // 조준 상태가 아닐 때 크로스헤어 스프레드를 보간한다.
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 10.f); // 사격 상태일 때 크로스헤어 스프레드를 보간한다.
			HUDPackage.CrosshairSpread = 0.5f+ CrosshairVelocityFactor - CrosshairAimFactor+CrosshairShootingFactor; // 크로스헤어 스프레드를 설정한다.
			TFHUD->SetHUDPackage(HUDPackage); // HUD 패키지를 설정한다.
		}
	}
}
// ============================================================
// [수류탄 투척] ThrowGrenade()
// ------------------------------------------------------------
// 기능 요약 : 
//   - 수류탄 투척 상태로 전환하고 애니메이션 및 HUD 갱신.
// 알고리즘 설명 : 
//   1. 리로드/비무장 상태 확인
//   2. CombatState를 ThrowingGrenade로 설정
//   3. 수류탄 애니메이션 재생 및 왼손 부착
//   4. 서버 권한 여부에 따라 RPC 처리
//   5. HUD의 수류탄 개수 감소
// ============================================================
void UCBComponent::ThrowGrenade()
{
	if (CombatState == ECombatState::ECS_Reloading) return; //리로드 상태에서는 수류탄을 던지지 않는다.
	if (Grenades == 0) return; //수류탄이 없으면 함수를 종료한다.
	if (EquippedWeapon == nullptr || Character == nullptr) return; //장착된 무기나 캐릭터가 없으면 함수를 종료한다.
	if (CombatState != ECombatState::ECS_Unoccupied) return; //전투 상태가 비어있지 않으면 수류탄을 던지지 않는다.
	CombatState = ECombatState::ECS_ThrowingGrenade;//전투 상태를 수류탄 던지기로 설정한다.
	if (Character) {
		Character->PlayThrowGrendadeMontage(); //캐릭터의 수류탄 던지기 몽타주를 재생한다.
		AttachActorToLeftHand(EquippedWeapon); //무기를 왼손에 부착한다.
		ShowAttachedGrenade(true); //캐릭터의 부착된 수류탄을 보이게 한다.
	}
	if (Character && !Character->HasAuthority()) {
		ServerThrowGrenade(); //서버에 수류탄 던지기 요청을 보낸다.
	}
	if(Character && Character->HasAuthority()) {
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades); //수류탄 개수를 감소시킨다.
		UpdateHUDGrenades(); //HUD의 수류탄 개수를 업데이트한다.
	}
}

void UCBComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return; //수류탄이 없으면 함수를 종료한다.
	CombatState = ECombatState::ECS_ThrowingGrenade;//전투 상태를 수류탄 던지기로 설정한다.
	if (Character) {
		Character->PlayThrowGrendadeMontage(); //캐릭터의 수류탄 던지기 몽타주를 재생한다.
		AttachActorToLeftHand(EquippedWeapon); //무기를 왼손에 부착한다.
		ShowAttachedGrenade(true); //캐릭터의 부착된 수류탄을 보이게 한다.
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades); //수류탄 개수를 감소시킨다.
	UpdateHUDGrenades(); //HUD의 수류탄 개수를 업데이트한다.
}
void UCBComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ATFPlayerController>(Character->Controller) : Controller; //컨트롤러를 가져온다.
	if (Controller) {
		Controller->SetHUDGrenadeCount(Grenades); //컨트롤러가 있다면 HUD에 수류탄 개수를 설정한다.
	}
}
void UCBComponent::OnRep_Grenades()
{
	UpdateHUDGrenades(); //HUD의 수류탄 개수를 업데이트한다.
}
void UCBComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled()) // 로컬 플레이어 컨트롤러인 경우에만 실행한다.
	{
		FHitResult HitResult; // 트레이스 결과를 저장할 변수
		TraceUnderCrosshairs(HitResult); // 매 프레임마다 화면 중앙 아래의 물체를 추적한다.
		HitTarget = HitResult.ImpactPoint; // 히트 타겟을 트레이스 결과의 충돌 지점으로 설정한다.
		SetHUDCrossharis(DeltaTime); // 매 프레임마다 HUD의 크로스헤어를 설정한다.
		InterpFOV(DeltaTime); // FOV를 보간한다.
	}
	if (EquippedWeapon) {
		UpdateHUDGrenades(); //HUD의 수류탄 개수를 업데이트한다.
	}
}
bool UCBComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr && !bisAiming && CombatState!=ECombatState::ECS_ThrowingGrenade && 
		CombatState!=ECombatState::ECS_Reloading); //장착된 무기와 보조 무기가 모두 존재하는지 확인한다.
}
void UCBComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType)) {
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo); //해당 무기 타입의 보유 탄약을 증가시킨다.
		UpdateCarriedAmmo(); //보유 탄약을 업데이트한다.
	}
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType) {
		Reload(); //장착된 무기가 비어있고, 해당 무기 타입과 같으면 리로드를 시도한다.
	}
}

