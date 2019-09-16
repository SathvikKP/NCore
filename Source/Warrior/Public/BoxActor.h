// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxActor.generated.h"

UCLASS()
class WARRIOR_API ABoxActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoxActor();

	//void Get_Owner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* BoxMesh;

	void ReceiveAnyDamage(float Damage, const class UDamageType * DamageType, class AController * InstigatedBy, AActor * DamageCauser);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float DefaultHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float HealthPercentage;

};
