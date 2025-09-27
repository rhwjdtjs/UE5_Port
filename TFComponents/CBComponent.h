#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnrealProject_7a/HUD/TFHUD.h"
#include "UnrealProject_7A/Weapon/WeaponTypes.h"
#include "CombatStates.h"
#include "CBComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_7A_API UCBComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCBComponent();
	friend class ATimeFractureCharacter;
	void EquipWeapon(class AWeapon* WeaponEquip); //무기를 장착하는 함수
	void SwapWeapons(); //무기를 교체하는 함수
	void EquippedWeaponPositionModify();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//복제하는 항목을 정의하는 함수
	UFUNCTION(BlueprintCallable)
	void FinishReload(); //재장전 완료 함수
	UFUNCTION(BlueprintCallable)
	void PushAllHUDFromCombat();
	UFUNCTION(Server, Reliable)
	void ServerFinishReload();
	void FireButtonPressed(bool bPressed); //발사 버튼이 눌렸을 때 호출되는 함수
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload(); //샷건 탄약 재장전 함수

	void JumpToShotgunEnd();
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished(); //수류탄 투척 완료 함수
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade(); //수류탄 발사 함수
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);
private:
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState=ECombatState::ECS_Unoccupied; //전투 상태
	UFUNCTION()
	void OnRep_CombatState(); //전투 상태가 변경되었을 때 호출되는 함수
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon; //장착된 무기
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon; //보조 무기
	UFUNCTION()
	void OnRep_SecondaryWeapon(); //보조 무기가 변경되었을 때 호출되는 함수
	class ATimeFractureCharacter* Character; //캐릭터
	class ATFPlayerController* Controller; //플레이어 컨트롤러
	class ATFHUD* TFHUD; //HUD
	UPROPERTY(Replicated)
	bool bisAiming; //조준 여부
	UPROPERTY(EditAnywhere, Category = "Combat")
	float baseWalkSpeed; //기본 걷는 속도
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimingWalkSpeed; //조준 상태의 걷는 속도

	bool bFireButtonPressed; //발사 버튼이 눌렸는지 여부
	//허브 & 크로스헤어
	FHUDPakage HUDPackage; //HUD 패키지 구조체
	float CrosshairVelocityFactor; //크로스헤어 속도 계수
	FVector  HitTarget; //히트 타겟
	float CrosshairAimFactor; //크로스헤어 조준 계수
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimFactorValue1 = 0.58f; //조준 계수 값 1
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AimFactorValue2 = 30.f; //조준 계수 값 1
	float CrosshairShootingFactor; //크로스헤어 사격 계수
	//FOV 에이밍
	//조준중이 아닐때 FOV
	float DefaultFOV; //기본 FOV
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV=30.f; //줌된 FOV
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed=20.f; //줌 인터폴레이션 속도
	UPROPERTY(EditAnywhere, Category = "Combat")
	float TargetDistance;
	float CurrentFOV; //현재 FOV
	void InterpFOV(float DeltaTime); //FOV를 보간하는 함수
	//자동화기
	FTimerHandle FireTimer; //발사 타이머 핸들
	void StartFireTimer(); //발사 타이머를 시작하는 함수
	void FireTimerFinished(); //발사 타이머가 끝났을 때 호출되는 함수
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bCanFire = true; //발사 가능 여부

	bool CanFire(); //발사 가능 여부를 확인하는 함수
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo; //보유 탄약
	UFUNCTION()
	void OnRep_CarriedAmmo(); //보유 탄약이 변경되었을 때 호출되는 함수
	void HandleReload(); //재장전 처리 함수
	int32 AmountToReload(); //재장전할 양
	void UpdateAmmoValues(); //탄약 값을 업데이트하는 함수
	void UpdateShotgunAmmoValues(); //샷건 탄약 값을 업데이트하는 함수
	void ShowAttachedGrenade(bool bShowGrenade); //부착된 수류탄을 표시하는 함수
	UPROPERTY(EditAnywhere)
	int32 StartingCarriedAmmo = 30; //시작 보유 탄약 수
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0; //시작 보유 탄약 수
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 60; //시작 보유 탄약 수
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 100; //시작 보유 탄약 수
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 14; //시작 보유 탄약 수
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 10; //시작 보유 탄약 수
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeAmmo = 10; //시작 보유 탄약 수
	UPROPERTY(ReplicatedUsing=OnRep_Grenades)
	int32 Grenades = 3; //수류탄 개수
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 9; //최대 수류탄 개수

	void UpdateHUDGrenades(); //HUD의 수류탄 개수를 업데이트하는 함수
	UFUNCTION()
	void OnRep_Grenades(); //수류탄 개수가 변경되었을 때 호출되는 함수
	void InitializeCarriedAmmo(); //보유 탄약을 초기화하는 함수
    // TMap은 언리얼 엔진의 템플릿 컨테이너로, 키-값 쌍을 저장하는 해시 맵입니다.
    // TMap<KeyType, ValueType> 형태로 사용되며, 키를 통해 값에 빠르게 접근할 수 있습니다.

    // 게임에서 TMap이 주로 사용되는 예시들:
    // 1. 인벤토리 시스템 - TMap<int32, FItemData> Inventory; (아이템 ID -> 아이템 정보)
    // 2. 플레이어 스탯 - TMap<FString, float> PlayerStats; ("Health" -> 100.0f, "Mana" -> 50.0f)
    // 3. 무기별 탄약 관리 - TMap<EWeaponType, int32> AmmoCount; (무기 타입 -> 탄약 수)
    // 4. 퀘스트 진행상황 - TMap<int32, bool> QuestProgress; (퀘스트 ID -> 완료 여부)
    // 5. 게임 설정 - TMap<FString, FString> GameSettings; ("Resolution" -> "1920x1080")

    // 현재 코드에서의 사용 예시:
	TMap<EWeaponType, int32> CarriedAmmoMap;  // 무기 타입별 보유 탄약을 저장하는 맵
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 999;
    /*
    무기 타입별 보유 탄약을 저장하는 맵
    - 키(Key): EWeaponType - 무기의 종류 (예: 권총, 소총, 샷건 등)
    - 값(Value): int32 - 해당 무기에 대한 보유 탄약 수

    사용 예시:
    CarriedAmmoMap.Add(EWeaponType::AssaultRifle, 120);  // 돌격소총 탄약 120발 추가
    CarriedAmmoMap.Add(EWeaponType::Pistol, 60);         // 권총 탄약 60발 추가

    int32 RifleAmmo = CarriedAmmoMap[EWeaponType::AssaultRifle]; // 돌격소총 탄약 조회
    CarriedAmmoMap[EWeaponType::Pistol] = 30;            // 권총 탄약을 30발로 변경

    // 주요 TMap 함수들:
    // - Add(Key, Value): 새로운 키-값 쌍 추가
    // - Remove(Key): 지정된 키의 항목 제거
    // - Find(Key): 키에 해당하는 값의 포인터 반환 (없으면 nullptr)
    // - Contains(Key): 키가 존재하는지 확인
    // - Num(): 저장된 항목 수 반환
    // - Empty(): 모든 항목 제거

    장점:
    - O(1) 평균 시간복잡도로 빠른 검색, 삽입, 삭제
    - 키를 통한 직관적인 데이터 접근
    - 동적 크기 조절

    단점:
    - 메모리 오버헤드 존재
    - 순서가 보장되지 않음 (순서가 필요하면 TSortedMap 사용)
    */
protected:
	virtual void BeginPlay() override;
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	UFUNCTION(Server, Reliable)
	void ServerReload(); //서버에서 재장전 요청을 처리하는 함수
	void SetAiming(bool bAiming); //조준 상태를 설정하는 함수
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming); //서버에서 조준 상태를 설정하는 함수
	UFUNCTION()
	void OnRep_EquippedWeapon(); //장착된 무기가 바뀔 때 호출되는 함수
	
	void DropEquippedWeapon(); //장착된 무기를 떨어뜨리는 함수
	void AttachActorToRightHand(AActor* ActorToAttach); //오른손에 액터를 부착하는 함수
	void AttachActorToLeftHand(AActor* ActorToAttach); //왼손에 액터를 부착하는 함수
	void AttachActorToBack(AActor* ActorToAttach); //등에 액터를 부착하는 함수
	void UpdateCarriedAmmo(); //보유 탄약을 업데이트하는 함수
	void PlayEquipSound(AWeapon* WeaponToEquip); //장착 사운드를 재생하는 함수
	void ReloadEmptyWeapon(); //빈 무기 리로드 함수
	void Fire();
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTargert); //서버에서 발사 버튼이 눌렸는지 여부를 설정하는 함수
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTargert); //발사 멀티캐스트 함수

	void TraceUnderCrosshairs(FHitResult& TraceHitResult); //조준선 아래의 물체를 추적하는 함수
	void SetHUDCrossharis(float DeltaTime); //HUD의 크로스헤어를 설정하는 함수
	void ThrowGrenade(); //수류탄 투척 함수
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade(); //서버에서 수류탄 투척 요청을 처리하는 함수
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass; //수류탄 클래스
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void Reload();
	FORCEINLINE int32	GetGrenades() const { return Grenades; } //수류탄 개수를 반환하는 함수
	FORCEINLINE bool IsAiming() const { return bisAiming; } //조준 여부를 반환하는 함수
	FORCEINLINE float SetBaseWalkSpeed(float Speed) { return baseWalkSpeed = Speed; } //기본 걷는 속도를 설정하는 함수
	FORCEINLINE float SetAimWalkSpeed(float Speed) { return AimingWalkSpeed = Speed; } //조준 상태의 걷는 속도를 설정하는 함수
	bool ShouldSwapWeapons(); //무기를 교체할 수 있는지 여부를 확인하는 함수
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount); //탄약을 줍는 함수
};
