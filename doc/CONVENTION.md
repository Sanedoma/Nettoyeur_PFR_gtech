# Convention de documentation — obligatoire pour tous les agents

Le but : que le développeur puisse **refaire seul** tout ce qu'un agent a fait, et **comprendre** pourquoi. La documentation est un support d'apprentissage d'Unreal Engine et du C++.

## 1. Ce qui doit être documenté — tout

| Type d'action | Ce qu'il faut écrire |
|---|---|
| Création d'un fichier | Chemin, rôle du fichier en une phrase, pourquoi il existe |
| Modification d'un fichier | Chemin, ce qui change, pourquoi |
| Code C++ | Comment c'est implémenté, fichier par fichier ; les principes et morceaux de code **pas encore expliqués** |
| Configuration (`.Build.cs`, `.ini`, `.uproject`, Project Settings, plugins) | Où, valeur avant → après, pourquoi |
| Action dans l'éditeur Unreal | Étapes **à la manière d'un humain** : panneau, menu, clic, nœud, paramètre, connexion |
| Action réalisée par script / MCP (EngineWire, Python) | **Retraduite en équivalent manuel.** On écrit les clics, jamais « j'ai lancé un script » |
| Compilation | Méthode (Build.bat / Live Coding), résultat, erreurs et leur correction |
| Test | Procédure exacte, résultat attendu, résultat obtenu |
| Git | Branche, commits (message) |

## 2. Style d'écriture

- Français, phrases simples, tutoiement autorisé.
- Ne jamais supposer qu'une notion est connue : si elle n'est pas dans `doc/notions.md`, on l'explique.
- Expliquer le **pourquoi**, pas seulement le **quoi**.
- Pour l'éditeur, nommer les éléments **exactement** comme dans l'interface (l'éditeur du développeur est en français : donner le nom français et, entre parenthèses, l'anglais si utile, ex. « Détails (Details) »).

### Exemple — à éviter
> J'ai exécuté un script qui a créé `BP_TacheSang` avec Gravite = 2.

### Exemple — attendu
> 1. Dans le **Content Browser**, ouvre le dossier `Content/Nettoyeur/Preuves`.
> 2. Clic droit dans le vide → **Blueprint Class**.
> 3. Dans la fenêtre, déplie **All Classes**, cherche `PreuveNettoyable`, sélectionne-la → **Select**.
> 4. Nomme l'asset `BP_TacheSang`, puis double-clique dessus pour l'ouvrir.
> 5. Bouton **Class Defaults** en haut → panneau **Details** → catégorie **Preuve** → `Gravite` = **2**.
> 6. **Compile** puis **Save**.

## 3. La règle « pas encore expliqué »

Avant d'expliquer une notion, l'agent ouvre `doc/notions.md` :
- **La notion y est** → il met simplement un lien : *Voir [UPROPERTY](notions.md#uproperty).*
- **Elle n'y est pas** → il l'explique dans son entrée de journal (bloc « Notion nouvelle ») **et** l'ajoute à `doc/notions.md` avec un lien vers son explication.

Une notion = un concept réutilisable (ex. `UPROPERTY`, `TObjectPtr`, Line Trace, Physics Handle, Subsystem, Delegate, Live Coding, reparentage de Blueprint…). Un morceau de code non trivial (ex. une formule, une boucle de recherche du sol) s'explique sur place même s'il n'est pas une « notion ».

## 4. Où écrire

- Journal de l'agent : `doc/agents/<agent>.md` — une entrée par tâche, **la plus récente en haut**.
- Fiche de tâche : `doc/taches/T00X-nom.md` — statut et critères cochés.
- Notions : `doc/notions.md`.
- Décisions d'architecture : `doc/decisions.md` (orchestrateur uniquement).

## 5. Modèle d'entrée de journal

```markdown
## T00X — <titre> — <AAAA-MM-JJ>

**Objectif :** ce que la tâche permet dans le jeu.
**Étape du GDD :** §11 étape N.
**Branche :** `tache/T00X-nom`

### 1. Fichiers créés / modifiés
- `Source/Nettoyeur/...` — créé | modifié — rôle / ce qui change.

### 2. Implémentation du code
Explication fichier par fichier : comment c'est construit, pourquoi ce choix.
Extraits de code commentés quand c'est utile.

> **Notion nouvelle — <nom>**
> Explication simple, avec un exemple. (Ajoutée à `doc/notions.md`.)

### 3. Configuration
- `<fichier ou réglage>` : avant → après — raison.

### 4. Étapes dans l'éditeur
1. ...
2. ...

### 5. Compilation et tests
- Compilation : méthode → résultat (erreurs rencontrées → correction).
- Test : procédure → attendu → obtenu.

### 6. Git
- Commits : `T00X: ...`

### 7. Résultat et points ouverts
- Ce qui marche, ce qui reste, questions pour l'orchestrateur.
```

Une section sans contenu s'écrit « Aucune » — elle n'est jamais supprimée, pour que le développeur sache qu'il n'y a rien à faire de ce côté.
