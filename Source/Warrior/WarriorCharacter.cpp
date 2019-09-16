// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "WarriorCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Arrow.h"
#include "BoxActor.h"
#include "Components/SphereComponent.h"

//////////////////////////////////////////////////////////////////////////
// AWarriorCharacter

AWarriorCharacter::AWarriorCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->SetupAttachment(RootComponent);



	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	//CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	
	AttackCount = 0;

	//Init health

	DefaultHealth = 100;
	Health = DefaultHealth;
	HealthPercentage = 1.0;


	//Detection Component
	
	// Use a sphere as a simple collision representation
	//RootComponent = GetCapsuleComponent();
	
	CollisionComp->InitSphereRadius(300.0f);
	//CollisionComp->BodyInstance.SetCollisionProfileName("Detection");
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AWarriorCharacter::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	//CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	//CollisionComp->CanCharacterStepUpOn = ECB_No;

	AttackOnOff = false;

	
	
}


//////////////////////////////////////////////////////////////////////////
// Input

void AWarriorCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AWarriorCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWarriorCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	/*
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AWarriorCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AWarriorCharacter::LookUpAtRate);
	*/
	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AWarriorCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AWarriorCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AWarriorCharacter::OnResetVR);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AWarriorCharacter::Attack);
}


void AWarriorCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AWarriorCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AWarriorCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AWarriorCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	//AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AWarriorCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	//AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AWarriorCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AWarriorCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

int i;
// Called when the game starts or when spawned
void AWarriorCharacter::BeginPlay()
{
	Super::BeginPlay();

	i = 0;
	if (Team == true)
	{
		FVector BoxPos = FVector(-1000, 1000, 200);
		UWorld* const World = GetWorld();
		const FRotator SpawnRotation = GetActorRotation();
		FActorSpawnParameters ActorSpawnParams;
		GetWorld()->SpawnActor<ABoxActor>(Box, BoxPos, SpawnRotation, ActorSpawnParams);
	}
}



void AWarriorCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// get first player pawn location
	//FVector MyCharacter = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	PlayerPosition = this->GetActorLocation();
	// screen log player location
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Player Location: %s"), *NewChar.ToString()));	
	//bool check = IsMoving();

	
	
	if (i == 5)
	{
		if (IsMoving() == true)
		{
			AttackCount = 0;

			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Reset the attack count"));
		}
	}

	i += 1;

	if (i > 11)
	{
		i = 1;
	}
	

	if (IsAttacking == false)
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		this->EnableInput(PlayerController);
	}

	/*
	if (IsMoving() == true )
	{
		AttackCount = 0;
	}
	*/
}

void AWarriorCharacter::Attack()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	this->DisableInput(PlayerController);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Attack Pressed"));
		//IsAttacking = true;
	}

	IsAttacking = true;
	AttackCount += 1;
	GoToSwitch();

	/*
	if (IsAttacking == true)
	{
		SaveAttack = true;
	}
	else
	{
		IsAttacking = true;
		GoToSwitch();
	}
	*/
	/*Line Trace*/

	FHitResult OutHit;


	FVector Initpos = this->GetActorLocation();
	FVector TempVector = this->GetActorForwardVector();
	FVector Finalpos = ((TempVector *100.f + Initpos));
	Finalpos = Finalpos + FVector(0, 0, 50);


	//FVector Start = this->GetActorLocation();
	

	// alternatively you can get the camera location
	// FVector Start = FirstPersonCameraComponent->GetComponentLocation();

	FVector Start = Finalpos;
	FVector ForwardVector = this->GetActorForwardVector();
	FVector End = ((ForwardVector * 2000.f) + Start);
	FCollisionQueryParams CollisionParams;

	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 5, 0, 1);

	if(GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_PhysicsBody, CollisionParams))
	{
		if(OutHit.bBlockingHit)
		{
            if (GEngine) {
				/*
			    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("You are hitting: %s"), *OutHit.GetActor()->GetName()));
			    GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Impact Point: %s"), *OutHit.ImpactPoint.ToString()));
                GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Normal Point: %s"), *OutHit.ImpactNormal.ToString()));
				*/
            }
		}
	}

	//projectile
	//UWorld* const World = GetWorld();

	//const FRotator SpawnRotation = GetActorRotation();
	//Set Spawn Collision Handling Override
	//FActorSpawnParameters ActorSpawnParams;
	//ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	//FVector SpawnLocation = Start;

	//World->SpawnActor<AArrow>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

	//SpawnProjectileArrow(Start);
}

void AWarriorCharacter::SpawnProjectileArrow()
{

	FVector Initpos = this->GetActorLocation();
	FVector TempVector = this->GetActorForwardVector();
	FVector Finalpos = ((TempVector *100.f + Initpos));
	Finalpos = Finalpos + FVector(0, 0, 50);

	FVector Start = Finalpos;

	//projectile
	UWorld* const World = GetWorld();

	const FRotator SpawnRotation = GetActorRotation();
	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	//ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	FVector SpawnLocation = Start;

	World->SpawnActor<AArrow>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);

	if (AttackCount == 3  && AttackOnOff == true)
	{
		count=2;
		GetWorld()->GetTimerManager().SetTimer(Delay, this, &AWarriorCharacter::TimerEnd, 0.2f, false);
	}

}

void AWarriorCharacter::TimerEnd()
{
	FVector Initpos = this->GetActorLocation();
	FVector TempVector = this->GetActorForwardVector();
	FVector Finalpos = ((TempVector *100.f + Initpos));
	Finalpos = Finalpos + FVector(0, 0, 50);

	FVector Start = Finalpos;

	//projectile
	UWorld* const World = GetWorld();

	const FRotator SpawnRotation = GetActorRotation();
	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	//ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	FVector SpawnLocation = Start;
	if (count != 0)
	{
		World->SpawnActor<AArrow>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		count -= 1;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta, FString::Printf(TEXT("Delay Test")));
		GetWorld()->GetTimerManager().SetTimer(Delay, this, &AWarriorCharacter::TimerEnd, 0.2f, false);
		AttackOnOff = false;
	}
	
}

void AWarriorCharacter::ComboAttackSave()
{
	if (SaveAttack == true)
	{
		SaveAttack = false;
		GoToSwitch();
	}
}

void AWarriorCharacter::GoToSwitch()
{
	//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	switch (AttackCount)
	{
	case 1:
		//AttackCount = 1;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Attack 1")));
		//Animation 1
		
		break;
	case 2:
		//AttackCount = 2;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Attack 2")));
		//Animation 1
		
		break;
	case 3:
		
		//AttackCount = 0;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Attack 3")));
		AttackOnOff = true;

		//ComboAnim
		
		
		break;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Error")));

	}
}

void AWarriorCharacter::ResetCombo()
{
	IsAttacking = false;
	//AttackCount = 0;
	SaveAttack = false;
}

bool AWarriorCharacter::IsMoving()
{
	//if (this->GetVelocity.SizeSquared() < 5)
	if(GetCharacterMovement()->Velocity.Size() == 0)
	{
		//character is not moving
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Player not moving")));
		return false;
	}
	else
	{
		//Character is moving
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Player Moving")));
		return true;

	}
}


float AWarriorCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	Health -= DamageAmount;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("TakeDamage Function Called in WarriorCharacter, Damage recieved"));

	if (Health <= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("Character Destroyed"));
		Destroy(this);
	}
	return Health;
}

void AWarriorCharacter::OnHit(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("Object detected : %s"), *OtherComp->GetName()));
	AWarriorCharacter* WarriorChar = Cast<AWarriorCharacter>(OtherActor);
	if (!WarriorChar) return;
	if ((OtherActor!=this) && OtherActor == WarriorChar && OtherComp != WarriorChar->CollisionComp && !Team)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Object detected is a WarriorCharacter: %s"), *OtherActor->GetName()));

		// Item is a weapon
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Object detected is a WarriorCharacter: %s"), *OtherActor->GetName()));

	}
}

bool AWarriorCharacter::ReturnTeam()
{
	return true;
}