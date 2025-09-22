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
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayWireEffects(const FVector& Start, const FVector& Target);
	UPROPERTY(EditDefaultsOnly, Category = "Wire|Effects")
	class UNiagaraSystem* WireShootEffect;  
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDrawWire(const FVector& Start, const FVector& End);
	UPROPERTY(EditDefaultsOnly, Category = "Wire|Effects")
	class UNiagaraSystem* WireImpactEffect;   // 충돌 파티클 (Niagara)
	UPROPERTY()
	class ATimeFractureCharacter* Character;
	UPROPERTY(ReplicatedUsing = OnRep_WireState)
	FVector WireTarget; //와이어 끝지점
	UPROPERTY(ReplicatedUsing = OnRep_WireState)
	bool bIsAttached = false; //와이어가 걸려있는지

	UPROPERTY(EditAnywhere, Category="Wire")
	float PullSpeed = 2000.f; //이동속도
	UPROPERTY(EditaNYWHERE, Category="Wire")
	float MaxWireDistance = 3000.f; //최대 와이어거리(라인트레이스)

	UPROPERTY(EditAnywhere, Category="Wire")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility; //라인트레이스 충돌채널
	UFUNCTION(Server, Reliable)
	void ServerFireWire();
	UFUNCTION(Server, Reliable)
	void ServerReleaseWire();

	UFUNCTION()
	void OnRep_WireState();
};
