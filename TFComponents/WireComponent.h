// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WireComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALPROJECT_7A_API UWireComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWireComponent();

	friend class ATimeFractureCharacter;

	void FireWire();
	void ReleaseWire();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintCallable, Category = "Wire")
	bool IsAttached() const { return bIsAttached; }
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UPROPERTY(EditAnywhere, Category = "Wire|Effects")
	class UNiagaraSystem* WireTravelEffect;   // ���ư��� ���� ������ ���� ����� ����Ʈ
	UFUNCTION(Client, Reliable)
	void ClientWireFail();
	UFUNCTION(Client, Reliable)
	void ClientWallFail();
	UFUNCTION(NetMulticast,Reliable)
	void MulticastWireSuccess();
	UPROPERTY()
	class UNiagaraComponent* ActiveTravelEffectLeft;
	UPROPERTY()
	class UNiagaraComponent* ActiveTravelEffectLeftFront;
	UPROPERTY()
	class UNiagaraComponent* ActiveTravelEffectRightFront;
	UPROPERTY()
	class UNiagaraComponent* ActiveTravelEffectRight;
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStartZipperSound();
	UPROPERTY(EditAnywhere, Category = "Wire")
	USoundBase* ZipperLoopSound;   // ���ư��� ���� ����� �Ҹ�
	UPROPERTY()
	UAudioComponent* ZipperAudioComponent;
	FTimerHandle CooldownUITimerHandle;
	float RemainingCooldown = 0.f;
	UPROPERTY(EditAnywhere, Category = "Wire")
	TSubclassOf<class UUserWidget> WireCooldownWidgetClass;
	UPROPERTY()
	class UUserWidget* WireCooldownWidget;
	void UpdateWireCooldownUI();
	void TickWireCooldownUI();
	UPROPERTY()
	class UTextBlock* WireCooldownText;
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayWireSound();
	void ResetWireCooldown();
	UPROPERTY(ReplicatedUsing = OnRep_CanFireWire)
	bool bCanFireWire = true;
	UFUNCTION()
	void OnRep_CanFireWire();
	UPROPERTY(EditAnywhere, Category = "Wire")
	float WireCooldown = 3.0f; // 3�� ��Ÿ��
	FTimerHandle CooldownTimerHandle;
	UPROPERTY(EditAnywhere, Category = "Wire")
	USoundBase* WireFireSound;
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayWireEffects(const FVector& Start, const FVector& Target);
 
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDrawWire(const FVector& Start, const FVector& End);
	UPROPERTY(EditDefaultsOnly, Category = "Wire")
	class UNiagaraSystem* WireImpactEffect;   // �浹 ��ƼŬ (Niagara)
	UPROPERTY()
	class ATimeFractureCharacter* Character;
	UPROPERTY(ReplicatedUsing = OnRep_WireState)
	FVector WireTarget; //���̾� ������
	UPROPERTY(ReplicatedUsing = OnRep_WireState)
	bool bIsAttached = false; //���̾ �ɷ��ִ���

	UPROPERTY(EditAnywhere, Category="Wire")
	float PullSpeed = 2500.f; //�̵��ӵ�
	UPROPERTY(EditaNYWHERE, Category="Wire")
	float MaxWireDistance = 10000.f; //�ִ� ���̾�Ÿ�(����Ʈ���̽�)

	UPROPERTY(EditAnywhere, Category="Wire")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility; //����Ʈ���̽� �浹ä��
	UFUNCTION(Server, Reliable)
	void ServerFireWire();
	UFUNCTION(Server, Reliable)
	void ServerReleaseWire();

	UFUNCTION()
	void OnRep_WireState();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopWireEffects();
};
