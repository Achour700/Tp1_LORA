# Nœud Collecteur LoRa

## Auteurs
- HOUIMEL Achour
- Najmi Ilyass

## Description
Ce projet implémente un nœud collecteur utilisant la technologie LoRa pour la communication sans fil avec d'autres nœuds. Il est conçu pour recevoir des demandes d'identification, gérer l'affectation des identifiants aux nœuds, recevoir des données de capteurs et gérer les sorties de nœuds du réseau.

## Utilisation avec Arduino Pro Mini ou une carte compatible "UCA"

1. Connectez la carte LoRa compatible à votre Arduino Pro Mini ou carte compatible "UCA".

2. Chargez le code du nœud collecteur sur la carte à l'aide de l'environnement de développement Arduino.

3. Assurez-vous de connecter les capteurs de température et d'humidité si vous souhaitez recevoir ces données.

4. Exécutez le programme et observez la communication avec les nœuds du réseau LoRa.

## Utilisation du programme Python pour visualiser les données

Le fichier Python inclus (`collecteur.py`) permet de visualiser en temps réel les données de température et d'humidité provenant du port série de la carte LoRa.

### Prérequis
- Python installé sur votre système
- Bibliothèques Python requises : `serial`, `matplotlib`

### Instructions
1. Assurez-vous que la carte LoRa est connectée à votre ordinateur via le port série.

2. Exécutez le fichier Python `visualiser_donnees.py` à l'aide d'un interpréteur Python. Assurez-vous d'installer les bibliothèques requises en utilisant la commande `pip install serial matplotlib` si ce n'est pas déjà fait.

3. Le programme Python détectera automatiquement le port série de la carte LoRa et commencera à visualiser les données en temps réel.

4. Fermez le programme Python en utilisant `Ctrl+C` dans le terminal.

