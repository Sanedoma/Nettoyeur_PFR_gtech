#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ComposantNettoyage.generated.h"

class UInteractionComponent;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnProgressionNettoyageChanged,
	float,
	Progression
);

/**
 * Nettoyage des preuves visées. Ne vise pas lui-même : il lit la cible de
 * l'UInteractionComponent du même acteur et réagit à son événement OnFocusChanged.
 * Une cible est nettoyable si son profil d'interactions autorise Wipe.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NETTOYEUR_API UComposantNettoyage : public UActorComponent
{
	GENERATED_BODY()

public:
	UComposantNettoyage();

	/** Widget du prompt, donné par le Blueprint via DefinirWidgetInteraction. */
	UPROPERTY(BlueprintReadOnly, Category = "Interface")
	TObjectPtr<UUserWidget> WidgetInteraction;

	UPROPERTY(BlueprintAssignable, Category = "Nettoyage")
	FOnProgressionNettoyageChanged OnProgressionNettoyageChanged;

	/** Écrit la progression dans le journal (Output Log) à chaque essuyage. Faux par défaut. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nettoyage|Debug")
	bool bAfficherDebug = false;

	/** Essuie la cible visée si son profil autorise Wipe (touche E). */
	UFUNCTION(BlueprintCallable, Category = "Nettoyage")
	void TenterNettoyage();

	UFUNCTION(BlueprintCallable, Category = "Interface")
	void DefinirWidgetInteraction(UUserWidget* Widget);

	/** Affiche ou masque le prompt en appelant la fonction AfficherInteraction du widget. */
	UFUNCTION()
	void MettreAJourTexteInteraction(bool Afficher);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Composant de visée du même acteur, trouvé une seule fois au BeginPlay. */
	UPROPERTY(Transient)
	TObjectPtr<UInteractionComponent> ComposantInteraction;

	/** Abonné à UInteractionComponent::OnFocusChanged (doit être une UFUNCTION). */
	UFUNCTION()
	void GererChangementCible(AActor* NouvelleCible);

	/** Vrai si une cible est visée et que son profil autorise Wipe. */
	bool PeutNettoyerCible() const;
};
