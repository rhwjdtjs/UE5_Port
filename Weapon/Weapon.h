

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8 {
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),
	EWS_MAX UMETA(DisplayName = "Default Max State"),
};

UCLASS()
class UNREALPROJECT_7A_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	void ShowPickupWidget(bool bShowPickupWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//복제하는 항목을 정의하는 함수
	virtual void Fire(const FVector& HitTarget); //발사 함수, 자식 클래스에서 구현할 수 있다.
	virtual void OnRep_Owner() override; //소유자가 변경될 때 호출되는 함수
	void DropWeapon(); //무기를 떨어뜨리는 함수
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	void OnShpereEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

public:	
	virtual void Tick(float DeltaTime) override;
	void SetWeaponState(EWeaponState State); //무기 상태를 설정하는 함수
	FORCEINLINE class USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; } //줌된 FOV를 반환하는 함수
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; } //줌 인터폴레이션 속도를 반환하는 함수
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; } //탄창 용량을 반환하는 함수
	//크로스헤어 텍스쳐
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsBottom;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsTop;
	//자동화기
	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireRate = 0.15f; //발사 속도
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutoMatickFire = true; //자동 발사 여부
	//줌 FOV 
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomedFOV = 30.f; //줌 시 FOV
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomInterpSpeed = 20.f; //줌 시 FOV 보간 속도
	void SetHUDAmmo();
private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere)
	EWeaponState WeaonState;
	UPROPERTY(EditAnywhere)
	class UAnimationAsset* FireAnimation; //발사 애니메이션
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;
	UFUNCTION()
	void OnRep_WeaponState();
	void SetWeaponStateMesh(EWeaponState State); //무기 상태에 따라 메쉬를 설정하는 함수
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass; //케이싱 클래스
	UPROPERTY(EditAnywhere, Category = "Weapon Ammo", ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;
	UPROPERTY(EditAnywhere, Category = "Weapon Ammo")
	int32 MagCapacity; //탄창 용량
	UFUNCTION()
	void OnRep_Ammo(); //탄약을 복제할 때 호출되는 함수
	
	

	void SpendRound(); //탄약을 소모하는 함수
	UPROPERTY()
	class ATimeFractureCharacter* TFOwnerCharacter;
	UPROPERTY()
	class ATFPlayerController* TFOwnerController; //플레이어 컨트롤러
};
