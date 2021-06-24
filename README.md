# Projet de Programmation Réseau   

<img alt="C" src="https://img.shields.io/badge/programming_language%20-%2300599C.svg?&style=flat-square&logo=c&logoColor=white"/>     

*Projet réalisé en binôme dans le cadre du cours "Programmation Réseaux"*   
**Auteurs : [Joël Hamilcaro](https://github.com/Joel-Hamilcaro/) et [Jie Tu](https://github.com/jie-tu)**   

Le but de se projet était de programmer un protocole réseau pour l'implémentation d'un "mur" de réseau social. Le protocole se base sur un algorithme non-fiable d'inondation, inspiré du protocole HNCP de Markus Stenberg. L'énoncé de ce projet précisait toutes les contraintes précises à respecter. Un serveur initial auquel notre programme devait se connecter était fourni par notre enseignant. Désormais, l'énoncé du projet et ce serveur initial ne sont plus accessibles.

## 1 Fonctionnalités du projet

Nous avons implémenté toutes les fonctionnalités de base
(sans extension) demandé par l'énoncé. Autrement dit, notre programme est capable de :
— Réaliser le protocole de maintenance de la liste des voisins (comme
indiqué dans la partie 4.2 de l'énoncé)
— Réaliser le protocole d’inondation (comme indiqué dans la partie 4.3 de l'énoncé)

## 2 Structure du programme et techniques employées

### 2.1 Structure du programme

Nous avons opté pour une approche modulaire pour notre programme.
Chaque fichier .c à un rôle précis : un pour les calculs (modulo, et calcul
de hash), un pour la gestion des TVL, un pour les voisins, etc.... Le code
source est dans le dossier src/ .
Un makefile est fourni pour effectuer la compilation du programme (Les
commandes sont précisées dans le fichier README.md.)

## 2.2 Techniques utilisées

### 2.2.1 Structure de données dynamiques

Pour la table des données et la table des voisins. Nous avons implé-
menté un module de liste doublement chaînée. (ils correspondent aux fi-
chiers list.c et list.h).

### 2.2.2 Bibliothèque de hashage

Pour calculer les différents hash, nous avons utilisé la bibliothèque
suivante : [https://github.com/DavidLeeds/hashmap](https://github.com/DavidLeeds/hashmap).  

### 2.2.3 Boucle à événement et nombres premiers

Nous utilisons une boucle à événement avec "select()" pour gérer les
différents événements du protocole. Pour envoyer une séquence de TVL
dans un paquet, nous utilisons un système basé sur l’unicité d’une facto-
risation en produits de facteurs premiers. En effet, on a une variable en-
tière "choix" qui détermine les TVL à envoyer dans notre séquence TVL.
De plus, chaque type de TVL est représenté par un nombre premier. Si on
veut ajouter un certain type de TVL à notre séquence, on multiplie "choix"
par le nombre premier. Par exemple, si choix==42, la séquence contiendra
au moins trois TVL dont les types sont respectivement représenté par les
nombres premiers : 2 , 3 et 7.

## 3 Principe de l’exécution du programme

A l’exécution du programme, nous initialisons tout d’abord un ID pour
notre noeud. L’ID est généré de manière aléatoire, et est unique (à très
forte probabilité).
Nous contactons un serveur initial (ici, jch.irif.fr au port 1212). A par-
tir de ce serveur, nous recherchons des pairs à contacter (nous envoyons
des TVL Neighbourg Request pour qu’il nous donne l’adresse de ses voi-
sins). Ces pairs sont ajoutés dans une liste de "voisins potentiels". Cette
liste n’est pas la "vraie" liste des voisins. En effet, on ajoute des pairs dans
vraie liste des voisins comme c’est indiqué dans le sujet (c’est-à-dire, après
avoir reçu un paquet correct de leurs part).
Ensuite, l’exécution du programme se fait comme indiqué dans le sujet.
Concernant l’envoi des données, on envoie constamment une donnée
de longueur nulle (comme c’est proposé dans le sujet). Mais parfois, on
envoie une donnée avec un message (au moment du rendu du projet, ce
message initialisé par la chaîne de caractère "OK").

## 4 Pistes d’améliorations

Ce projet répond aux consignes de base du sujet. Pour l’améliorer, il
faudrait implémenter les extensions qui ont été suggérés dans l'énoncé.
Enfin, nous pourrions ajouter un système plus interactif dans notre programme. Par exemple,
pouvoir demander à l’utilisateur sur quel serveur il veut s’initialiser, lui
donner la possibilité de taper directement les messages à envoyer via les
TVL Node State.

## Annexe : Instructions


#### Modifier le serveur initial

Lignes 41 et 42 dans src/client.c

#### Compiler

`$ make`

#### Exécuter

`$ ./run`

#### Pour exécuter en mode "dégub", il faut ajouter un argument, par exemple :

`$ ./run 1`

#### Nettoyer les fichiers binaires

`$ make clean`
