// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShooterCharacterBase.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Weapon/Weapon.h"
#include "Component/CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShooterPlayerController.h"
#include "Game/GotchaGameMode.h"
#include "Gotcha/Gotcha.h"
#include "CableComponent.h"

AShooterCharacterBase::AShooterCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetMesh(), FName("Head"));
	Camera->bUsePawnControlRotation = true;

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	AssistArea = CreateDefaultSubobject<USphereComponent>(TEXT("AssistArea"));
	AssistArea->SetupAttachment(GetCapsuleComponent());
	AssistArea->SetSphereRadius(128.f);
	AssistArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AssistArea->SetCollisionObjectType(ECC_WorldDynamic);
	AssistArea->SetCollisionResponseToAllChannels(ECR_Ignore);
	AssistArea->SetCollisionResponseToChannel(ECC_Assist, ECR_Block);
	
	Hook = CreateDefaultSubobject<UCableComponent>(TEXT("Hook"));
	Hook->SetupAttachment(GetCapsuleComponent());
	Hook->EndLocation = FVector::ZeroVector;
	Hook->CableLength = 1.f;
	Hook->SetVisibility(false);
	Hook->SetIsReplicated(true);
	
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_StaticMesh, ECR_Ignore);

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	GetMesh()->SetCollisionResponseToChannel(ECC_Assist, ECR_Block);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	GetCharacterMovement()->MaxWalkSpeed = 1200.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 600.f;
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
}

void AShooterCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	check(ShooterContext);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ShooterContext, 0);
		}
	}

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AShooterCharacterBase::ReceiveDamage);
		EquipWeapon();
	}
}

void AShooterCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollInit();

	if (HasAuthority())
	{
		CheckGrapple();
	}
	
	DoGrapple();
}

void AShooterCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Move);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AShooterCharacterBase::MoveButtonReleased);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Look);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Jump);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::CrouchButtonPressed);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Dash);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Fire);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacterBase::FireButtonReleased);
	EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Reload);
	EnhancedInputComponent->BindAction(SwapAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::SwapWeapons);
	EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Parry);
	EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Grapple);
}

void AShooterCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}
}

void AShooterCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterCharacterBase, Health);
	DOREPLIFETIME(AShooterCharacterBase, bDisableGameplay);
	DOREPLIFETIME(AShooterCharacterBase, GrabPoint);
	DOREPLIFETIME(AShooterCharacterBase, bIsGrappling);
	DOREPLIFETIME(AShooterCharacterBase, bCanGrapple);
}

void AShooterCharacterBase::PollInit()
{
	if (ShooterPlayerController == nullptr)
	{
		ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
		if (ShooterPlayerController)
		{
			UpdateHUDHealth();
		}
	}
}

void AShooterCharacterBase::CheckGrapple()
{
	if (!bIsGrappling) return;

	FVector Direction = (GrabPoint - Camera->GetComponentLocation()).GetSafeNormal();
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Direction * HookLength;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult GrappleResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
			GrappleResult,
			Start,
			End,
			ECC_Visibility,
			QueryParams
		);

	if (GetActorLocation().Equals(GrabPoint, 100.f) || (bHit && !GrappleResult.ImpactPoint.Equals(GrabPoint, 10.f)))
	{
		bIsGrappling = false;
		Hook->SetVisibility(false);
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}
}

void AShooterCharacterBase::DoGrapple()
{
	if (!bIsGrappling) return;

	Hook->EndLocation = Hook->GetComponentTransform().InverseTransformPosition(GrabPoint);

	FVector Direction = (GrabPoint - GetActorLocation()).GetSafeNormal();
	FVector Force = Direction * GrappleForce;
	GetCharacterMovement()->AddForce(Force);
}

void AShooterCharacterBase::Move(const FInputActionValue& InputActionValue)
{
	if (bDisableGameplay) return;
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	AddMovementInput(ForwardDirection, InputAxisVector.Y);
	AddMovementInput(RightDirection, InputAxisVector.X);
	JumpDirection = ForwardDirection * InputAxisVector.Y + RightDirection * InputAxisVector.X + FVector::UpVector;
	DashDirection = ForwardDirection * InputAxisVector.Y + RightDirection * InputAxisVector.X;
}

void AShooterCharacterBase::MoveButtonReleased()
{
	if (bDisableGameplay) return;
	
	if (Camera)
	{
		JumpDirection = FVector::UpVector;
		DashDirection = Camera->GetForwardVector();
		DashDirection.Z = 0.f;
	}
}

void AShooterCharacterBase::Look(const FInputActionValue& InputActionValue)
{
	if (bDisableGameplay) return;
	
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();
	
	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void AShooterCharacterBase::Jump()
{
	if (bDisableGameplay) return;
	
	if (bIsCrouched)
	{
		UnCrouch();
		return;
	}

	if (bIsGrappling)
	{
		bIsGrappling = false;
		Hook->SetVisibility(false);
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		return;
	}
		
	if (JumpCount >= 1 && JumpCount < MaxJumpCount && !CanJump())
	{
		if (JumpDirection == FVector::ZeroVector)
		{
			JumpDirection = FVector::UpVector;
		}
		const FVector JumpForce = JumpDirection * GetCharacterMovement()->JumpZVelocity;
		ServerLaunchCharacter(JumpForce);
		JumpCount++;
	}
	else if (CanJump())
	{
		Super::Jump();
		JumpCount = 1;
	}
}

void AShooterCharacterBase::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AShooterCharacterBase::Dash()
{
	if (bDisableGameplay) return;
	
	if (Camera && DashCount < MaxDashCount)
	{
		SetCollisionBetweenCharacter(ECR_Ignore);
		
		if (GetVelocity() == FVector::ZeroVector)
		{
			DashDirection = Camera->GetForwardVector();
			DashDirection.Z = 0.f;
		}
		DashDirection.Normalize();
		DashDirection *= DashForce;
		ServerLaunchCharacter(DashDirection);

		FTimerHandle EnableCollisionHandle;
		FTimerDelegate EnabledCollisionDelegate;
		EnabledCollisionDelegate.BindUFunction(this, FName("SetCollisionBetweenCharacter"), ECR_Block);
		GetWorldTimerManager().SetTimer(EnableCollisionHandle, EnabledCollisionDelegate, 0.1f, false);

		if (GetCharacterMovement()->IsFalling())
		{
			FTimerHandle StopHandle;
			GetWorldTimerManager().SetTimer(StopHandle, this, &AShooterCharacterBase::DashFinished, 0.1f);
		}
		
		DashCount++;
		if (!GetWorldTimerManager().IsTimerActive(DashHandle))
		{
			GetWorldTimerManager().SetTimer(DashHandle, this, &AShooterCharacterBase::DashReset, DashCoolTime);
		}
	}
}

void AShooterCharacterBase::ServerLaunchCharacter_Implementation(const FVector_NetQuantize& LaunchForce)
{
	LaunchCharacter(LaunchForce, true, true);
}

void AShooterCharacterBase::DashFinished()
{
	ServerDashFinished();
}

void AShooterCharacterBase::ServerDashFinished_Implementation()
{
	GetCharacterMovement()->StopMovementImmediately();
}

void AShooterCharacterBase::DashReset()
{
	DashCount--;
	if (DashCount > 0)
	{
		GetWorldTimerManager().SetTimer(DashHandle, this, &AShooterCharacterBase::DashReset, DashCoolTime);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString(TEXT("Dash Filled!")));
	}
}

void AShooterCharacterBase::Fire()
{
	if (bDisableGameplay) return;
	
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AShooterCharacterBase::FireButtonReleased()
{
	if (bDisableGameplay) return;
	
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AShooterCharacterBase::Reload()
{
	if (bDisableGameplay) return;
	
	if (Combat)
	{
		Combat->Reload();
	}
}

void AShooterCharacterBase::SwapWeapons()
{
	if (bDisableGameplay) return;
	
	if (Combat)
	{
		Combat->SwapWeapons();
	}
}

void AShooterCharacterBase::Parry()
{
	if (bDisableGameplay) return;

	if (Combat)
	{
		Combat->ParryButtonPressed();
	}
}

void AShooterCharacterBase::Grapple()
{
	if (bDisableGameplay) return;
	
	if (bCanGrapple)
	{
		ServerGrapple();
	}
}

void AShooterCharacterBase::ServerGrapple_Implementation()
{
	FVector Direction = GetBaseAimRotation().Vector();
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Direction * HookLength;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	FHitResult GrappleResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
			GrappleResult,
			Start,
			End,
			ECC_StaticMesh,
			QueryParams
		);

	if (bHit && !GrappleResult.ImpactNormal.Equals(FVector::UpVector))
	{
		UnCrouch();
		GrabPoint = GrappleResult.ImpactPoint;
		bIsGrappling = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		Hook->SetVisibility(true);
		bCanGrapple = false;

		FTimerHandle GrappleTimer;
		GetWorldTimerManager().SetTimer(
			GrappleTimer,
			this,
			&AShooterCharacterBase::GrappleFinished,
			GrappleCoolTime
		);
	}
}

void AShooterCharacterBase::GrappleFinished()
{
	bCanGrapple = true;
}

void AShooterCharacterBase::PlayFireMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_Katana:
			SectionName = FName("Katana");
			break;
		default:
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AShooterCharacterBase::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	GotchaGameMode = GotchaGameMode == nullptr ? GetWorld()->GetAuthGameMode<AGotchaGameMode>() : GotchaGameMode;
	if (bElimmed || GotchaGameMode == nullptr) return;

	if (Combat)
	{
		AWeapon* DamageCausedWeapon = Cast<AWeapon>(DamageCauser);
		if (Combat->Parry(DamageCausedWeapon)) return;
	}
	
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	
	if (Health == 0.f)
	{
		if (GotchaGameMode)
		{
			ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
			AShooterPlayerController* AttackerController = Cast<AShooterPlayerController>(InstigatorController);
			GotchaGameMode->PlayerEliminated(this, ShooterPlayerController, AttackerController);
		}
	}
}

void AShooterCharacterBase::Elim(bool bPlayerLeftGame)
{
	DestroyWeapons();
	MulticastElim(bPlayerLeftGame);
}

void AShooterCharacterBase::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	bElimmed = true;

	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AShooterCharacterBase::ElimTimerFinished,
		ElimDelay
	);
}

void AShooterCharacterBase::ElimTimerFinished()
{
	GotchaGameMode = GotchaGameMode == nullptr ? GetWorld()->GetAuthGameMode<AGotchaGameMode>() : GotchaGameMode;
	if (GotchaGameMode && !bLeftGame)
	{
		GotchaGameMode->RequestRespawn(this, Controller);
	}
}

void AShooterCharacterBase::DestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			Combat->EquippedWeapon->Destroy();
		}
		if (Combat->SecondaryWeapon)
		{
			Combat->SecondaryWeapon->Destroy();
		}
	}
}

void AShooterCharacterBase::EquipWeapon()
{
	if (Combat == nullptr) return;
	
	if (PrimaryGunClass && SecondaryGunClass)
	{
		AWeapon* PrimaryGun = GetWorld()->SpawnActor<AWeapon>(PrimaryGunClass);
		AWeapon* SecondaryGun = GetWorld()->SpawnActor<AWeapon>(SecondaryGunClass);
		if (PrimaryGun && SecondaryGun)
		{
			Combat->EquipWeapons(PrimaryGun, SecondaryGun);
		}
	}
}

void AShooterCharacterBase::SetCollisionBetweenCharacter(const ECollisionResponse NewResponse)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, NewResponse);
	ServerSetCollisionBetweenCharacter(NewResponse);
}

void AShooterCharacterBase::ServerSetCollisionBetweenCharacter_Implementation(const ECollisionResponse NewResponse)
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, NewResponse);
}

void AShooterCharacterBase::OnRep_Health()
{
	UpdateHUDHealth();
}

void AShooterCharacterBase::UpdateHUDHealth()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if (ShooterPlayerController)
	{
		ShooterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

AWeapon* AShooterCharacterBase::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}
