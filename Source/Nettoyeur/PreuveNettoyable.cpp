
#include "PreuveNettoyable.h"

#include "Components/StaticMeshComponent.h"
#include "InteractionProfileComponent.h"
#include "InteractionTypes.h"

APreuveNettoyable::APreuveNettoyable()
{
    PrimaryActorTick.bCanEverTick = false;

    MeshPreuve = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("MeshPreuve"));

    SetRootComponent(MeshPreuve);

    ProfilInteractions =
        CreateDefaultSubobject<UInteractionProfileComponent>(
            TEXT("ProfilInteractions"));

    ProfilInteractions->AllowedInteractions.Add(
        EInteractionType::Wipe);
}

void APreuveNettoyable::Essuyer()
{
    if (EstNettoyee())
    {
        return;
    }

    ProgressionNettoyage = FMath::Clamp(
        ProgressionNettoyage + ProgressionParEssuyage,
        0.0f,
        1.0f);

    MettreAJourEtatNettoyage();
}

bool APreuveNettoyable::EstNettoyee() const
{
    return ProgressionNettoyage >= 1.0f;
}

void APreuveNettoyable::MettreAJourEtatNettoyage()
{
    if (EstNettoyee())
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
    }
}
