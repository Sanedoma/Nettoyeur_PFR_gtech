// Fill out your copyright notice in the Description page of Project Settings.


#include "ComposantNettoyage.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PreuveNettoyable.h"

// Sets default values for this component's properties
UComposantNettoyage::UComposantNettoyage()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UComposantNettoyage::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UComposantNettoyage::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UComposantNettoyage::TenterNettoyage()
{
    Personnage = Cast<ACharacter>(GetOwner());

    if (!Personnage)
    {
        return;
    }

    CameraJoueur = Personnage->FindComponentByClass<UCameraComponent>();

    if (!CameraJoueur)
    {
        return;
    }

    const FVector Debut = CameraJoueur->GetComponentLocation();
    const FVector Direction = CameraJoueur->GetForwardVector();
    const FVector Fin = Debut + (Direction * 300.0f);

    FHitResult Resultat;
    FCollisionQueryParams Parametres;
    Parametres.AddIgnoredActor(Personnage);

    GetWorld()->LineTraceSingleByChannel(
        Resultat,
        Debut,
        Fin,
        ECC_Visibility,
        Parametres
    );

    if (Resultat.bBlockingHit)
    {
        APreuveNettoyable* Preuve =
            Cast<APreuveNettoyable>(Resultat.GetActor());

        if (Preuve)
        {
            Preuve->Essuyer();
        }
    }

    DrawDebugLine(
        GetWorld(),
        Debut,
        Fin,
        FColor::Green,
        false,
        2.0f,
        0,
        1.5f
    );
}