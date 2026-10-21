#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreuveNettoyable.generated.h"

class UStaticMeshComponent;
class UInteractionProfileComponent;

UCLASS()
class NETTOYEUR_API APreuveNettoyable : public AActor
{
    GENERATED_BODY()

public:
    APreuveNettoyable();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preuve")
    TObjectPtr<UStaticMeshComponent> MeshPreuve;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preuve")
    TObjectPtr<UInteractionProfileComponent> ProfilInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nettoyage",
        meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float ProgressionParEssuyage = 0.25f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nettoyage",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ProgressionNettoyage = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "Nettoyage")
    void Essuyer();

    UFUNCTION(BlueprintPure, Category = "Nettoyage")
    bool EstNettoyee() const;

private:
    void MettreAJourEtatNettoyage();
};
