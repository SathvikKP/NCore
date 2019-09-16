// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxActor.h"

// Sets default values
ABoxActor::ABoxActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box Mesh Text"));

	DefaultHealth = 100;
	Health = DefaultHealth;
	HealthPercentage = 1.0;


}

// Called when the game starts or when spawned
void ABoxActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABoxActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ReceiveAnyDamage(float Damage, const class UDamageType * DamageType, class AController * InstigatedBy, AActor * DamageCauser)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, TEXT("RecieveAnyDamage function called"));
}

float ABoxActor::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	Health -= DamageAmount;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("TakeDamage Function Called, Damage recieved"));

	if (Health <= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("Actor Destroyed"));
		this->Destroy();
	}
	return Health;
}