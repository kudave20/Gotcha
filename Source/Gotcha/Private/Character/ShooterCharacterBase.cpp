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
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"
#include "Game/GotchaGameState.h"
#include "Game/TeamGameMode.h"
#include "Interface/InteractableInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerState.h"
#include "Weapon/Flag.h"

AShooterCharacterBase::AShooterCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetMesh(), FName("head"));
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

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
	
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
		EquipWeapons();
	}
}

void AShooterCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset();

	if (HasAuthority())
	{
		CheckGrapple();
		CheckMantle();
	}
	
	DoGrapple();
}

void AShooterCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacterBase::Look);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AShooterCharacterBase::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AShooterCharacterBase::JumpButtonReleased);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AShooterCharacterBase::CrouchButtonPressed);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AShooterCharacterBase::Dash);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacterBase::Fire);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacterBase::FireButtonReleased);
	EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AShooterCharacterBase::Reload);
	EnhancedInputComponent->BindAction(SwapAction, ETriggerEvent::Started, this, &AShooterCharacterBase::SwapWeapons);
	EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Started, this, &AShooterCharacterBase::Parry);
	EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Started, this, &AShooterCharacterBase::Grapple);
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AShooterCharacterBase::Interact);
	EnhancedInputComponent->BindAction(RespawnAction, ETriggerEvent::Started, this, &AShooterCharacterBase::Respawn);
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
	DOREPLIFETIME(AShooterCharacterBase, GrabPoint);
	DOREPLIFETIME(AShooterCharacterBase, bIsGrappling);
	DOREPLIFETIME(AShooterCharacterBase, bCanGrapple);
	DOREPLIFETIME(AShooterCharacterBase, bIsMantling);
	DOREPLIFETIME(AShooterCharacterBase, JumpCount);
}

void AShooterCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsLocallyControlled())
	{
		UpdateHUDHealth();
		UpdateHUDOwnerTeam();
		UpdateHUDLeaderTeam();
		UpdateHUDOwnerRank();
	}
}

void AShooterCharacterBase::OnInteract(APlayerController* Player)
{
	if (bElimmed)
	{
		GotchaGameMode = GotchaGameMode == nullptr ? GetWorld()->GetAuthGameMode<AGotchaGameMode>() : GotchaGameMode;
		ATeamGameMode* TeamGameMode = Cast<ATeamGameMode>(GotchaGameMode);
		if (TeamGameMode)
		{
			MulticastRespawnImmediately();
		}
	}
}

void AShooterCharacterBase::MulticastRespawnImmediately_Implementation()
{	
	bElimmed = false;
	bDisableGameplay = false;
	Health = MaxHealth;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	EquipWeapons();
	UpdateHUDHealth();
}

void AShooterCharacterBase::AimOffset()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
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
			ECC_StaticMesh,
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

void AShooterCharacterBase::CheckMantle()
{
	if (!bJumpButtonHeld || bIsMantling)
	{
		return;
	}

	FVector Direction = GetBaseAimRotation().Vector();
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Direction * MantleLength;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult MantleResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(
			MantleResult,
			Start,
			End,
			ECC_StaticMesh,
			QueryParams
		);

	if (bHit)
	{
		FVector SphereStart = MantleResult.Location + FVector::UpVector * MantleHeight;

		TArray<AActor*> ActorsToIgnore;
		FHitResult SphereResult;
		bool bDetected = UKismetSystemLibrary::SphereTraceSingle(
				this,
				SphereStart,
				MantleResult.Location,
				10.f,
				UEngineTypes::ConvertToTraceType(ECC_StaticMesh),
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None,
				SphereResult,
				true
			);

		if (bDetected)
		{
			FVector FirstMantlePoint = SphereResult.ImpactPoint;
			FVector SecondMantlePoint = SphereResult.ImpactPoint + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	
			FVector DeltaZ = FVector::UpVector * GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			FVector TraceStart = SecondMantlePoint + FVector::UpVector * (GetCapsuleComponent()->GetScaledCapsuleRadius() + 1.f);
			FVector TraceEnd = SecondMantlePoint + DeltaZ * 2.f - FVector::UpVector * (GetCapsuleComponent()->GetScaledCapsuleRadius() - 1.f);

			FHitResult TraceResult;
			bool bSuccess = UKismetSystemLibrary::SphereTraceSingle(
				this,
				TraceStart,
				TraceEnd,
				GetCapsuleComponent()->GetScaledCapsuleRadius(),
				UEngineTypes::ConvertToTraceType(ECC_StaticMesh),
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None,
				TraceResult,
				true
			);

			if (!bSuccess)
			{
				bIsMantling = true;
				SecondMantlePoint.Z = SphereResult.ImpactPoint.Z;
				MulticastDoMantle(FirstMantlePoint, SecondMantlePoint);
			}
		}
	}
}

void AShooterCharacterBase::MulticastDoMantle_Implementation(const FVector_NetQuantize& FirstPoint, const FVector_NetQuantize& SecondPoint)
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	FMotionWarpingTarget FirstTarget;
	FirstTarget.Name = FName("FirstMantlePoint");
	FirstTarget.Location = FirstPoint;
	FirstTarget.Rotation = GetActorRotation();
	MotionWarping->AddOrUpdateWarpTarget(FirstTarget);

	FMotionWarpingTarget SecondTarget;
	SecondTarget.Name = FName("SecondMantlePoint");
	SecondTarget.Location = SecondPoint;
	SecondTarget.Rotation = GetActorRotation();
	MotionWarping->AddOrUpdateWarpTarget(SecondTarget);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && MantleMontage)
	{
		AnimInstance->Montage_Play(MantleMontage);

		FTimerHandle MantleTimer;
		GetWorldTimerManager().SetTimer(
			MantleTimer,
			this,
			&AShooterCharacterBase::MantleFinished,
			MantleMontageLength
			);
	}
}

void AShooterCharacterBase::MantleFinished()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	bIsMantling = false;
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

	ServerJumpButtonPressed(LastControlInputVector);

	if (CanJump())
	{
		Super::Jump();
	}
}

void AShooterCharacterBase::ServerJumpButtonPressed_Implementation(const FVector_NetQuantize& InputVector)
{
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
	
	bJumpButtonHeld = true;
	
	if (JumpCount >= 1 && JumpCount < MaxJumpCount && !CanJump())
	{
		FVector JumpDirection = InputVector;
		JumpDirection.Z = 1.f;
		FVector JumpForce = JumpDirection * GetCharacterMovement()->JumpZVelocity;
		LaunchCharacter(JumpForce, true, true);
		++JumpCount;
	}
	else if (CanJump())
	{
		JumpCount = 1;
	}
}

void AShooterCharacterBase::JumpButtonReleased()
{
	if (bDisableGameplay) return;
	
	ServerReleaseJumpButton();
}

void AShooterCharacterBase::ServerReleaseJumpButton_Implementation()
{
	bJumpButtonHeld = false;
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
		
		FVector DashDirection = LastControlInputVector.Equals(FVector::ZeroVector) ? Camera->GetForwardVector() : LastControlInputVector;
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

void AShooterCharacterBase::Interact()
{
	if (bDisableGameplay) return;
	
	ServerInteract();
}

void AShooterCharacterBase::ServerInteract_Implementation()
{
	FVector Direction = GetBaseAimRotation().Vector();
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Direction * InteractLength;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	
	FHitResult InteractResult;
	GetWorld()->LineTraceSingleByChannel(
			InteractResult,
			Start,
			End,
			ECC_Visibility,
			QueryParams
		);

	IInteractableInterface* Interactable = Cast<IInteractableInterface>(InteractResult.GetActor());
	if (Interactable)
	{
		ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
		Interactable->OnInteract(ShooterPlayerController);
	}
}

void AShooterCharacterBase::Respawn()
{
	if (bCanRespawn)
	{
		ServerRespawn();
	}
}

void AShooterCharacterBase::ServerRespawn_Implementation()
{
	GotchaGameMode = GotchaGameMode == nullptr ? GetWorld()->GetAuthGameMode<AGotchaGameMode>() : GotchaGameMode;
	if (GotchaGameMode && !bLeftGame)
	{
		GotchaGameMode->RequestRespawn(this, Controller);
	}
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
	
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AShooterCharacterBase::ElimTimerFinished,
		ElimDelay
	);
}

void AShooterCharacterBase::ElimTimerFinished()
{
	bCanRespawn = true;
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

void AShooterCharacterBase::EquipWeapons()
{
	if (Combat == nullptr || !HasAuthority()) return;
	
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

void AShooterCharacterBase::HoldFlag(AFlag* Flag)
{
	if (Flag == nullptr || Combat == nullptr) return;

	Combat->bHoldingFlag = true;
	Flag->SetOwner(this);

	/*
	 * Play animations and do somethings for holding the flag.
	 */
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

void AShooterCharacterBase::ClientResetForTeamRespawn_Implementation()
{
	GetWorldTimerManager().ClearTimer(ElimTimer);
	bCanRespawn = false;
}

void AShooterCharacterBase::UpdateHUDHealth()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if (ShooterPlayerController)
	{
		ShooterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AShooterCharacterBase::UpdateHUDAmmo()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if (ShooterPlayerController && Combat && Combat->EquippedWeapon)
	{
		ShooterPlayerController->SetHUDAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void AShooterCharacterBase::UpdateHUDOwnerTeam()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if (ShooterPlayerController)
	{
		AShooterPlayerState* SPState = GetPlayerState<AShooterPlayerState>();
		if (SPState)
		{
			ShooterPlayerController->SetHUDOwnerTeam(SPState->GetTeam());
		}
	}
}

void AShooterCharacterBase::UpdateHUDLeaderTeam()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if (ShooterPlayerController)
	{
		AGotchaGameState* GGameState = Cast<AGotchaGameState>(UGameplayStatics::GetGameState(this));
		if (GGameState)
		{
			ShooterPlayerController->SetHUDLeaderTeam(GGameState->LeaderTeam);
		}
	}
}

void AShooterCharacterBase::UpdateHUDOwnerRank()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if (ShooterPlayerController)
	{
		AShooterPlayerState* SPState = GetPlayerState<AShooterPlayerState>();
		if (SPState)
		{
			ShooterPlayerController->SetHUDOwnerRank(SPState->GetTeamRank());
		}
	}
}

AWeapon* AShooterCharacterBase::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

bool AShooterCharacterBase::IsHoldingFlag() const
{
	if (Combat == nullptr) return false;
	return Combat->bHoldingFlag;
}

void AShooterCharacterBase::SetHoldingFlag(bool bHolding)
{
	if (Combat == nullptr) return;
	Combat->bHoldingFlag = bHolding;
}
