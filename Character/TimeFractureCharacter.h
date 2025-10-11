#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UnrealProject_7A/Interfaces/InteractWithCrosshairInterface.h"
#include "UnrealProject_7A/TFComponents/CombatStates.h"
#include "TimeFractureCharacter.generated.h"

/**
 * ATimeFractureCharacter
 * -------------------------------------------------------------
 * 플레이어 캐릭터의 이동, 전투, 상호작용, UI, 네트워크 복제 등
 * 전반적인 기능을 담당하는 메인 클래스.
 */
UCLASS()
class UNREALPROJECT_7A_API ATimeFractureCharacter
	: public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	/* ============================ */
	/*   기본 시스템 함수 섹션    */
	/* ============================ */
	ATimeFractureCharacter();											 // 생성자 - 컴포넌트 초기화
	virtual void Tick(float DeltaTime) override;						 // 매 프레임 업데이트
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // 입력 바인딩 설정
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 네트워크 복제 등록
	virtual void PostInitializeComponents() override;					 // 컴포넌트 초기화 이후 세팅

	void Elim();														 // 캐릭터 제거 처리
	void UpdateHUDHealth();											     // HUD 체력 갱신
	void UpdateHUDShield();											     // HUD 실드 갱신

	/* ============================ */
	/*   멀티캐스트 및 이벤트     */
	/* ============================ */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();												 // 사망 연출을 모든 클라에 전파

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;										 // 조작 비활성화 여부

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);						 // 스나이퍼 조준경 표시/숨김 BP 이벤트

	bool bisElimmed = false;											 // 캐릭터가 제거된 상태인지 여부

	void PlayThrowGrendadeMontage();									 // 수류탄 투척 애니메이션 재생
	void HandleChatKey();												 // 채팅창 열기
	void HandleChatCancel();											 // 채팅 입력 취소
	void HandleChatSubmit();											 // 채팅 전송

	UFUNCTION(BlueprintImplementableEvent)
	void BloodScreen();												     // 피격 시 화면 이펙트 BP에서 구현

	UFUNCTION(Client, Reliable)
	void ClientShowBloodScreen();										 // 클라 전용 블러드 이펙트 RPC

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Wire", meta = (AllowPrivateAccess = "true"))
	class UWireComponent* WireComponent;								 // 와이어(그래플링 훅 등) 기능 컴포넌트


protected:
	/* ============================ */
	/*   생명주기 및 입력 처리    */
	/* ============================ */
	virtual void Jump() override;										 // 점프 입력
	virtual void Destroyed() override;									 // 오브젝트 파괴 시 처리
	virtual void BeginPlay() override;									 // 게임 시작 시 초기화
	void SwapButtonPressed();											 // 무기 교체 입력
	virtual void PossessedBy(AController* NewController) override;		 // 컨트롤러가 캐릭터를 소유할 때 호출

	UFUNCTION(Server, Reliable)
	void ServerSwapButtonPressed();										 // 서버 측 무기 교체 처리

	void Dive();														 // 회피(구르기) 동작
	UFUNCTION(Server, Reliable)
	void ServerDivePressed();											 // 서버로 구르기 요청
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDive();												 // 구르기 애니메이션 전체 전파

	// 이동 및 카메라 입력 처리
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	// 행동 입력
	void EquipButton();												     // 무기 장착
	void CrouchButton();												 // 앉기/해제
	void AimButton();													 // 조준 시작
	void AimButtonRelease();											 // 조준 해제
	void AimOffset(float DeltaTime);									 // 조준 보정 계산 (Yaw/Pitch)
	void FireButtonPressed();											 // 발사 시작
	void FireButtonReleased();											 // 발사 중지
	void ReloadButtonPressed();											 // 재장전
	void GrenadeButtonPressed();										 // 수류탄 투척
	void WireButtonPressed();											 // 와이어 발사
	void WireButtonReleased();											 // 와이어 해제

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		class AController* InstigatorController, AActor* DamageCursor);	 // 데미지 처리

	void PollInit();													 // PlayerState/HUD 초기화 체크
	virtual void OnRep_PlayerState() override;							 // PlayerState 복제 후 처리


private:
	/* ============================ */
	/*   내부 로직 및 변수 섹션  */
	/* ============================ */
	UFUNCTION(Server, Reliable)
	void ServerSetActorRotation(const FRotator& NewRotation);			 // 서버 회전 동기화

	// --- 조준 관련 ---
	float AO_YAW;														 // 현재 조준 Yaw
	float AO_PITCH;													     // 현재 조준 Pitch
	FRotator BaseAimRotation;											 // 기준 조준 회전값

	// --- 카메라 관련 ---
	UPROPERTY(VisibleAnywhere, Category = "camera")
	class USpringArmComponent* CameraBoom;								 // 카메라 붐 (거리 조절용)
	UPROPERTY(VisibleAnywhere, Category = "camera")
	class UCameraComponent* FollowCamera;								 // 실제 시점 카메라
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;								 // 캐릭터 머리 위 위젯 (닉네임 등)

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;									 // 현재 충돌 중인 무기

	// --- 카메라 오프셋 ---
	UPROPERTY(EditAnywhere, Category = "camera")
	FVector CrouchingCameraOffset;										 // 앉은 상태 카메라 위치
	UPROPERTY(EditAnywhere, Category = "camera")
	FVector NormalOffset;												 // 기본 카메라 위치
	UPROPERTY(EditAnywhere, Category = "camera")
	float AimCameraOffset;												 // 조준 시 거리
	UPROPERTY(EditAnywhere, Category = "camera")
	float normalAimCameraOffset;										 // 일반 조준 거리
	UPROPERTY(EditAnywhere, Category = "camera")
	float CameraInterpSpeed = 15.f;										 // 카메라 보간 속도

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);					 // 무기 겹침 변경 시 처리

	// --- 전투 관련 컴포넌트 ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	class UCBComponent* CombatComponent;								 // 전투 컴포넌트
	UPROPERTY(VisibleAnyWhere)
	class UBuffComponent* BuffComponent;								 // 버프 및 스탯 보정 컴포넌트

	UFUNCTION(Server, Reliable)
	void ServerEquipButton();											 // 서버 측 장착 처리

	// --- 수류탄 ---
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* AttachedGrenade;							 // 캐릭터에 부착된 수류탄 메쉬

	// --- 전투 애니메이션 몽타주 ---
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ThrowGrenadeMontage;							 // 수류탄 던지기
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;								 // 발사
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;								 // 피격 반응
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ElimMontage;									 // 사망
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadMontage;									 // 재장전
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* DiveMontage;									 // 구르기

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;										 // 근접 시 메쉬 숨김 거리
	void HideCameraIfCharacterClose();									 // 카메라 근접 시 숨기기

	// --- 체력 및 실드 ---
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MaxHealth = 100.f;											 // 최대 체력
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player State")
	float Health = 100.f;												     // 현재 체력
	UPROPERTY(EditAnywhere, Category = "Player State")
	float MaxShield = 100.f;											 // 최대 실드
	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player State")
	float Shield = 0.f;												     // 현재 실드

	UFUNCTION()
	void OnRep_Shield(float LastShield);								 // 실드 변경 시 호출
	UPROPERTY(Replicated)
	FRotator MoveRotation;												 // 이동 회전값 복제

	UPROPERTY(EditAnywhere)
	float RollDistance = 200.f;										     // 구르기 이동 거리

	FTimerHandle ElimTimer;												 // 사망 후 리스폰 타이머 핸들
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;												 // 리스폰 대기 시간
	void ElimTimerFinished();											 // 타이머 종료 시 호출

	UFUNCTION()
	void OnRep_Health(float LastHealth);								 // 체력 변경 시 호출

	class ATFPlayerController* TfPlayerController;						 // 플레이어 컨트롤러 참조
	UPROPERTY()
	class ATFPlayerState* TfPlayerState;								 // 플레이어 상태 참조

	// --- 기본 무기 ---
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;							 // 시작 시 지급 무기 클래스

	UPROPERTY(EditAnywhere)
	class USoundCue* HitCharacterSound;								 // 피격 시 사운드
	UPROPERTY(EditAnywhere)
	USoundCue* HitConfirmSound;										     // 공격 명중 사운드

	UFUNCTION(NetMulticast, UnReliable)
	void MulticastHitCharacterSound();									 // 피격 사운드 전파


public:
	/* ============================ */
	/*  🌐 상태 및 접근자 함수 섹션 */
	/* ============================ */
	bool bIsDodging;													 // 회피 중 여부

	void EnsureOverheadWidgetLocal();									 // 오버헤드 위젯 초기화
	void RefreshOverheadName();										     // 머리 위 닉네임 새로고침

	void SpawnDefaultWeapon();											 // 기본 무기 생성
	void UpdateHUDAmmo();												 // HUD 탄약 갱신
	void SetOverlappingWeapon(AWeapon* Weapon);							 // 무기 겹침 설정

	bool IsWeaponEquipped();											 // 무기 장착 여부
	bool IsAiming();													 // 조준 여부

	// --- 인라인 접근자 ---
	FORCEINLINE float GETAO_YAW() const { return AO_YAW; }				 // 현재 조준 Yaw
	FORCEINLINE float GETAO_PITCH() const { return AO_PITCH; }			 // 현재 조준 Pitch
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool IsElimmed() const { return bisElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE UCBComponent* GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE USkeletalMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }

	// --- 기능성 반환 함수 ---
	class UOverheadWidget* GetOverheadWidget() const;					 // 오버헤드 위젯 반환
	ECombatState GetCombatState() const;								 // 전투 상태 반환
	AWeapon* GetEquippedWeapon();										 // 장착 무기 반환

	void PlayFireMontage(bool bAiming);								 // 발사 애니메이션 재생
	void PlayElimMontage();											     // 사망 애니메이션 재생
	void PlayHitReactMontage();										     // 피격 반응 재생
	void PlayReloadMontage();											 // 재장전 애니메이션 재생

	UFUNCTION(NetMulticast, UnReliable)
	void MultiCastHit();												 // 피격 애니 전파

	FVector GetHitTarget() const;										 // 조준 대상 위치 반환
};
