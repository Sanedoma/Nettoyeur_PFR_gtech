
#include "PersonnageNettoyeur.h"
#include "Components/InputComponent.h"

APersonnageNettoyeur::APersonnageNettoyeur()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APersonnageNettoyeur::BeginPlay()
{
    Super::BeginPlay();
}

void APersonnageNettoyeur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APersonnageNettoyeur::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}
void APersonnageNettoyeur::TenterNettoyage()
{
    // La logique du rayon sera ajoutée à l'étape suivante.
}