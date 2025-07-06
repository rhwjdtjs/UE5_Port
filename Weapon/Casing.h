// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class UNREALPROJECT_7A_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACasing();
private:
	UPROPERTY(VisibleAnywhere, Category = "Casing Properties")
	UStaticMeshComponent* CasingMesh; //케이싱 메쉬
	UPROPERTY(EditAnywhere, Category = "Casing Properties")
	float ShellEjectionImpluse;
	UPROPERTY(EditAnywhere, Category="Casing Properties")
	class USoundCue* ShellEjectionSound; //케이싱 발사 사운드
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
