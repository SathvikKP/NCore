// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WarriorCharacter.generated.h"

UCLASS(config=Game)
class AWarriorCharacter : public ACharacter
{
	GENERATED_BODY()

		
	

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AWarriorCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	
	UFUNCTION(BlueprintCallable)
	void Attack();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool IsAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool SaveAttack;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int AttackCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FVector PlayerPosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FVector AttackPosition;

	UFUNCTION(BlueprintCallable)
	void ComboAttackSave();

	UFUNCTION(BlueprintCallable)
	void ResetCombo();

	UFUNCTION(BlueprintCallable)
	void GoToSwitch();

	UFUNCTION(BlueprintCallable)
		bool IsMoving();

	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float offset;
	

	virtual void Tick(float DeltaTime) override;


	//Bullet

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	
	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	
	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AArrow> ProjectileClass;

	UFUNCTION(BlueprintCallable)
	void SpawnProjectileArrow();

	//Box Actor for health

	UPROPERTY(EditAnywhere, Category= Box)
	TSubclassOf<class ABoxActor> Box;

	//Health properties

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float DefaultHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		float HealthPercentage;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser);


	//Detection properties

	/** Sphere collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Detectotherobjects)
	class USphereComponent* CollisionComp;
	
	/** called when projectile hits something */
	
	UFUNCTION()
	void OnHit(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	//team
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool Team;

	UFUNCTION(BlueprintCallable)
		bool ReturnTeam();

	//delay

	UPROPERTY(EditAnywhere)
		FTimerHandle Delay;

	UFUNCTION(BlueprintCallable)
		void TimerEnd();

	UPROPERTY(EditAnywhere)
		int count;

	UPROPERTY(EditAnywhere)
		bool AttackOnOff;
};