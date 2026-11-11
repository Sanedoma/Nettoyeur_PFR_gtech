# NETTOYEUR — Instructions pour Claude Code

Ce fichier est lu automatiquement par Claude Code à chaque session ouverte dans ce dépôt.
La session principale joue le rôle d'**orchestrateur** ; les sous-agents sont définis dans `.claude/agents/`.

---

## 1. Le projet

- **Jeu :** NETTOYEUR — simulation immersive FPS. Le joueur est un nettoyeur professionnel qui intervient *après* un crime : il nettoie, déplace et dissimule des preuves avant une inspection qui calcule un score de détection.
- **Moteur :** Unreal Engine **5.7.4** (installé dans `C:/Program Files/Epic Games/UE_5.7`). **Ne jamais migrer de version** sans décision explicite dans `doc/decisions.md`.
- **Langages :** C++ pour la logique, Blueprints pour la configuration et le contenu.
- **Plateforme :** PC Windows, clavier/souris.
- **Développeur :** projet solo d'un étudiant qui **apprend** Unreal. La documentation produite par les agents est un support d'apprentissage : elle est aussi importante que le code.

## 2. Documents de référence (à lire avant toute tâche)

| Fichier | Contenu |
|---|---|
| `doc/gdd/GDD_Nettoyeur_Complet.md` | Game Design Document : **référence principale** (vision, systèmes, architecture cible, plan en étapes) |
| `doc/gdd/Boucle-de-Gameplay.txt` | Idées initiales et vision long terme |
| `doc/00_etat-projet.md` | Ce qui existe **réellement**, ce qui marche, la prochaine tâche |
| `doc/decisions.md` | Décisions d'architecture déjà prises |
| `doc/notions.md` | Index des notions déjà expliquées dans la doc |
| `doc/CONVENTION.md` | **Convention de documentation obligatoire** |
| `doc/taches/` | Fiches des tâches |

Ne jamais confondre ce que le GDD **décrit** avec ce qui est **implémenté**. Seul `doc/00_etat-projet.md` fait foi sur l'état réel.

## 3. Équipe d'agents

| Agent | Fichier | Périmètre |
|---|---|---|
| Orchestrateur | (session principale) | Découpe, attribue, vérifie, tient `00_etat-projet.md`, `decisions.md`, les fiches de tâche et le GDD à jour |
| `gameplay-cpp` | `.claude/agents/gameplay-cpp.md` | Code C++ des systèmes : interaction, physique, preuves, score, inspection |
| `editeur-contenu` | `.claude/agents/editeur-contenu.md` | Blueprints enfants, inputs, widgets UMG, niveaux, Data Assets, réglages éditeur |
| `qa` | `.claude/agents/qa.md` | Compilation, tests en jeu, vérification des critères, relecture code + doc. **N'écrit jamais le code qu'il vérifie** |
| `rendu` | `.claude/agents/rendu.md` | Matériaux, decals, lampe UV, Niagara, éclairage (Bloc 2 — pas avant que les preuves soient stables) |

Chaque agent tient **son propre journal** dans `doc/agents/<nom>.md`. L'orchestrateur tient `doc/agents/orchestrateur.md`.

## 4. Workflow d'une tâche

1. **Fiche** — l'orchestrateur crée ou met à jour `doc/taches/T00X-nom.md` (objectif, étape du GDD, périmètre de fichiers, critères d'acceptation, agent assigné).
2. **Branche** — `git checkout -b tache/T00X-nom` depuis `main` à jour.
3. **Implémentation** — l'agent assigné travaille **uniquement** dans le périmètre de la fiche.
4. **Journal** — l'agent écrit son entrée dans `doc/agents/<nom>.md` selon `doc/CONVENTION.md`, et complète `doc/notions.md`.
5. **Vérification** — l'agent `qa` compile, teste, coche les critères dans la fiche et écrit son rapport dans `doc/agents/qa.md`. En cas d'échec, la tâche retourne à l'agent avec la liste précise des problèmes.
6. **Validation humaine** — le développeur teste lui-même dans Unreal. **Aucune tâche n'est terminée sans sa validation.**
7. **Intégration** — commit(s) `T00X: <résumé>`, merge dans `main` après validation, push.
8. **Mise à jour** — l'orchestrateur met à jour `00_etat-projet.md`, `decisions.md` et le GDD si une décision a changé.

**Un seul module à la fois.** Jamais deux agents sur les mêmes fichiers en même temps. L'orchestrateur ne lance la tâche suivante que lorsque la précédente est validée et mergée.

## 5. Règles de développement

- **Examiner l'existant avant de modifier.** Ne jamais réécrire un système sans l'avoir lu en entier.
- **Préserver ce qui fonctionne.** Le nettoyage actuel (`UComposantNettoyage` + `APreuveNettoyable`) fonctionne en jeu : toute évolution doit garder ce comportement (ou le faire évoluer explicitement selon la fiche).
- **Pas de doublons.** Chercher (Grep) si une fonction ou classe équivalente existe avant d'en créer une.
- **Pas de fonctionnalité hors fiche.** Ne pas élargir le périmètre (pas de van, IA, poursuites, boutique, pièges avant que le GDD le prévoie).
- **C++ simple et lisible.** Pas d'abstraction tant qu'un deuxième cas concret ne la justifie pas (GDD §10.2).
- **La logique en C++, la configuration en Blueprint.** Les graphes Blueprint ne sont pas scriptables de façon fiable : si un graphe est indispensable, l'agent le **décrit nœud par nœud** dans sa doc et c'est le développeur qui le monte.
- **Données réglables dans l'éditeur** : `UPROPERTY(EditAnywhere, ...)` avec `Category` et, si utile, `ClampMin/ClampMax`.
- **Séparer état et apparence** d'une preuve (GDD §5.2).
- **Encodage :** fichiers sources en **UTF-8**. Identifiants C++ sans accents ; commentaires en français autorisés.
- **Nommage :** préfixes Unreal (`A`, `U`, `F`, `E`, `I`), assets `BP_`, `WBP_`, `M_`, `MI_`, `IA_`, `IMC_`, `DA_`, `L_`.

## 6. Compilation et tests

- **Éditeur fermé :** compiler en ligne de commande :
  ```
  "C:/Program Files/Epic Games/UE_5.7/Engine/Build/BatchFiles/Build.bat" NettoyeurEditor Win64 Development -Project="C:/Users/ouz_s/Documents/Unreal Projects/Nettoyeur_PFR_gtech/Nettoyeur.uproject" -WaitMutex
  ```
- **Éditeur ouvert :** Live Coding est actif et bloque la compilation externe. L'agent **demande au développeur** de lancer Live Coding (`Ctrl+Alt+F11`) ou de fermer l'éditeur. Un changement de `.h` qui ajoute/retire des `UPROPERTY`/`UFUNCTION`/classes impose de **fermer l'éditeur et recompiler**.
- **Tests en jeu :** décrits dans la fiche de tâche. Si les outils EngineWire (MCP Unreal) sont disponibles, l'agent peut lancer Play, lister les acteurs et prendre des captures ; sinon il fournit au développeur la procédure de test pas à pas.
- **Logs :** `Saved/Logs/Nettoyeur.log` (lecture seule, jamais versionné).

## 7. Git

- Branche par tâche : `tache/T00X-nom`. Messages : `T00X: <verbe à l'infinitif> <quoi>`.
- **Jamais** de commit direct sur `main`, de `push --force`, ni de réécriture d'historique.
- Ne jamais versionner `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` (déjà dans `.gitignore`).
- Les `.uasset` / `.umap` passent par **Git LFS** (`.gitattributes`).
- Avant toute modification risquée (renommage de classe C++, reparentage de Blueprint, suppression d'asset) : commit propre préalable.

## 8. Documentation — résumé (détail obligatoire dans `doc/CONVENTION.md`)

**Toute action est documentée** : création ou modification de fichier, code, configuration, action dans l'éditeur, compilation, test, Git.

- **Code :** expliquer comment c'est implémenté et pourquoi ; expliquer chaque principe ou morceau de code **pas encore expliqué** (vérifier `doc/notions.md` d'abord, sinon lien).
- **Éditeur :** décrire les étapes **comme un humain les ferait** (menu, clic, nœud, paramètre, connexion), jamais « j'ai exécuté un script ».
- **Action réalisée par script / MCP :** toujours retraduite en équivalent manuel.

## 9. Interdits

- Prétendre qu'une fonctionnalité marche sans l'avoir testée.
- Supprimer ou renommer un asset ou une classe existante sans que la fiche le demande.
- Modifier `doc/gdd/` sans passer par l'orchestrateur.
- Toucher à `Saved/`, `Intermediate/`, `Binaries/`, `DerivedDataCache/`.
