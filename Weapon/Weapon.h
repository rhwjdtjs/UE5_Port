

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	//�����ϴ� �׸��� �����ϴ� �Լ�
	virtual void Fire(const FVector& HitTarget); //�߻� �Լ�, �ڽ� Ŭ�������� ������ �� �ִ�.
	virtual void OnRep_Owner() override; //�����ڰ� ����� �� ȣ��Ǵ� �Լ�
	void DropWeapon(); //���⸦ ����߸��� �Լ�
	void AddAmmo(int32 AmmoToAdd); //ź���� �߰��ϴ� �Լ�
	bool bDestroyWeapon = false; //���⸦ �ı����� ����
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
	//����� ���� ����
	void EnableCustomDepth(bool bEnable);
public:	
	virtual void Tick(float DeltaTime) override;
	void SetWeaponState(EWeaponState State); //���� ���¸� �����ϴ� �Լ�
	FORCEINLINE class USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; } //�ܵ� FOV�� ��ȯ�ϴ� �Լ�
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; } //�� ���������̼� �ӵ��� ��ȯ�ϴ� �Լ�
	FORCEINLINE int32 GetAmmo() const { return Ammo; } //���� ź���� ��ȯ�ϴ� �Լ�
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; } //źâ �뷮�� ��ȯ�ϴ� �Լ�
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; } //���� Ÿ���� ��ȯ�ϴ� �Լ�
	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound; //���� ���� ����
	//ũ�ν���� �ؽ���
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
	//�ڵ�ȭ��
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float FireRate = 0.15f; //�߻� �ӵ�
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bAutoMatickFire = true; //�ڵ� �߻� ����
	//�� FOV 
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomedFOV = 30.f; //�� �� FOV
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomInterpSpeed = 20.f; //�� �� FOV ���� �ӵ�
	void SetHUDAmmo();
	bool IsEmpty(); //ź���� ������ Ȯ���ϴ� �Լ�
	bool IsFull(); //ź���� ���� á���� Ȯ���ϴ� �Լ�
private:
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType; //���� Ÿ��
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere)
	EWeaponState WeaonState;
	UPROPERTY(EditAnywhere)
	class UAnimationAsset* FireAnimation; //�߻� �ִϸ��̼�
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;
	UFUNCTION()
	void OnRep_WeaponState();
	void SetWeaponStateMesh(EWeaponState State); //���� ���¿� ���� �޽��� �����ϴ� �Լ�
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass; //���̽� Ŭ����
	UPROPERTY(EditAnywhere, Category = "Weapon Properties", ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	int32 MagCapacity; //źâ �뷮
	UFUNCTION()
	void OnRep_Ammo(); //ź���� ������ �� ȣ��Ǵ� �Լ�
	
	

	void SpendRound(); //ź���� �Ҹ��ϴ� �Լ�
	UPROPERTY()
	class ATimeFractureCharacter* TFOwnerCharacter;
	UPROPERTY()
	class ATFPlayerController* TFOwnerController; //�÷��̾� ��Ʈ�ѷ�
};
