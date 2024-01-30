// Fill out your copyright notice in the Description page of Project Settings.


#include "Control/ControlPoint.h"
#include "Character/ShooterCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Game/ControlGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerState.h"

AControlPoint::AControlPoint()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Area = CreateDefaultSubobject<UBoxComponent>(TEXT("Area"));
	Area->SetupAttachment(Mesh);
	Area->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Area->SetCollisionObjectType(ECC_WorldStatic);
	Area->SetCollisionResponseToAllChannels(ECR_Ignore);
	Area->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AControlPoint::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Area->OnComponentBeginOverlap.AddDynamic(this, &AControlPoint::OnSphereBeginOverlap);
		Area->OnComponentEndOverlap.AddDynamic(this, &AControlPoint::OnSphereEndOverlap);
	}
}

void AControlPoint::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacterBase* ShooterCharacter = Cast<AShooterCharacterBase>(OtherActor);
	if (ShooterCharacter)
	{
		AControlGameMode* GameMode = Cast<AControlGameMode>(UGameplayStatics::GetGameMode(this));
		AShooterPlayerState* SPState = ShooterCharacter->GetPlayerState<AShooterPlayerState>();
		if (GameMode)
		{
			GameMode->ControlPoint(SPState, nullptr);
		}
	}
}

void AControlPoint::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AShooterCharacterBase* ShooterCharacter = Cast<AShooterCharacterBase>(OtherActor);
	if (ShooterCharacter)
	{
		AControlGameMode* GameMode = Cast<AControlGameMode>(UGameplayStatics::GetGameMode(this));
		AShooterPlayerState* SPState = ShooterCharacter->GetPlayerState<AShooterPlayerState>();
		if (GameMode)
		{
			GameMode->ControlPoint(nullptr, SPState);
		}
	}
}

