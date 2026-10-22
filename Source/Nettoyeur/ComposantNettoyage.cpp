// Fill out your copyright notice in the Description page of Project Settings.

#include "ComposantNettoyage.h"
#include "UObject/StructOnScope.h"
#include "UObject/UnrealType.h"

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

    Personnage = Cast<ACharacter>(GetOwner());

    if (!Personnage || !GetWorld())
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
    bool PeutNettoyer = false;
    
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
            PeutNettoyer = true;
        }
    }
    MettreAJourTexteInteraction(PeutNettoyer);

    DrawDebugLine(
        GetWorld(),
        Debut,
        Fin,
        FColor::Green,
        false,
        0.0f,
        0,
        1.5f
    );
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

            const float Progression = Preuve->ProgressionNettoyage;

            UE_LOG(LogTemp, Warning, TEXT("Progression nettoyage : %f"), Progression);

            OnProgressionNettoyageChanged.Broadcast(Progression);

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

void UComposantNettoyage::DefinirWidgetInteraction(UUserWidget* Widget)
{
    WidgetInteraction = Widget;
}

void UComposantNettoyage::MettreAJourTexteInteraction(bool Afficher)
{
    if (!WidgetInteraction)
    {
        return;
    }

    UFunction* Fonction = WidgetInteraction->FindFunction(
        FName("AfficherInteraction")
    );

    if (Fonction)
    {
        struct FParametresInteraction
        {
            bool Afficher;
        };

        FParametresInteraction Parametres;
        Parametres.Afficher = Afficher;

        WidgetInteraction->ProcessEvent(Fonction, &Parametres);
    }
}