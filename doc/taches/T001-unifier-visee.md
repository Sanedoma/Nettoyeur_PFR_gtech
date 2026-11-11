# T001 — Unifier la visée et faire passer le nettoyage par le profil d'interaction

**Statut :** À faire (dépend de T000)
**Agent :** `gameplay-cpp`
**Branche :** `tache/T001-unifier-visee`
**Étape du GDD :** §11 étape 2, §6.1 « Principe confirmé : interactions indépendantes et assignables »

## Objectif
Une seule source de vérité pour « ce que le joueur vise » : `UInteractionComponent`. Le nettoyage utilise cette cible et vérifie que la cible autorise l'interaction `Wipe`, au lieu de faire son propre rayon et un `Cast` direct.

## Travail attendu
1. `UInteractionComponent` expose la cible courante (ex. `GetCurrentFocus()`) et notifie son changement (ex. delegate `OnFocusChanged`), pour que le prompt ne soit plus recalculé à chaque frame par un autre composant.
2. `UComposantNettoyage` :
   - n'effectue plus de Line Trace propre ; il utilise la cible de `UInteractionComponent` ;
   - vérifie `CanPerform(EInteractionType::Wipe)` sur le profil de la cible avant d'essuyer ;
   - conserve exactement le comportement actuel : +0.25 par essuyage, delegate `OnProgressionNettoyageChanged`, mise à jour du prompt ;
   - le `DrawDebugLine` passe derrière un booléen réglable `bAfficherDebug` (faux par défaut).
3. Supprimer la fonction vide `APersonnageNettoyeur::TenterNettoyage()` **seulement si** T000 confirme qu'aucun Blueprint ne l'appelle ; sinon la signaler.
4. Retirer `GetAimDirection()` si inutilisé.

## Hors périmètre
- Le remplacement du `ProcessEvent` du prompt (sera traité avec l'UI de feedback).
- Le système de preuves (T003).

## Fichiers autorisés
`InteractionComponent.h/.cpp`, `ComposantNettoyage.h/.cpp`, `PersonnageNettoyeur.h/.cpp`.

## Critères d'acceptation
- [ ] Un seul trace/sweep de visée par frame dans tout le projet.
- [ ] Viser une `BP_PreuveNettoyable` affiche le prompt ; ne plus la viser le masque.
- [ ] Essuyer 4 fois fait passer la progression 0.25 → 0.5 → 0.75 → 1.0 et la preuve disparaît (comme avant).
- [ ] Un objet **sans** `Wipe` dans son profil ne peut pas être essuyé et n'affiche pas le prompt de nettoyage.
- [ ] Pas de ligne de debug à l'écran par défaut ; `bAfficherDebug = true` la réaffiche.
- [ ] Grab et Pull inchangés.
- [ ] Compilation sans nouvel avertissement.
- [ ] Journal conforme à `doc/CONVENTION.md` (notamment : delegate, binding, pourquoi supprimer un double trace).
