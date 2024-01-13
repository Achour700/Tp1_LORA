# Auteurs: HOUIMEL Achour, Najmi Illyas
# Ce programme visualise les données de température et d'humidité provenant d'un port série.

import serial
import matplotlib.pyplot as plt
import serial.tools.list_ports

def recup_port_board():
    # Récupère le port série de la carte
    ports = list(serial.tools.list_ports.comports())
    mData = None

    for p in ports:
        if "USB-SERIAL" in p.description or "USB2.0-Serial" in p.description:
            mData = serial.Serial(p.device, 9600, timeout=10)

        print(p.device)
    return mData

# Récupère le port série
ser = recup_port_board()

# Dictionnaire pour stocker les données par nœud
data_by_node = {}

# Crée une figure avec deux sous-graphiques pour température et humidité
fig, (ax_temp, ax_humid) = plt.subplots(2, 1, sharex=True)

plt.ion()  # Active le mode interactif pour la mise à jour dynamique du graphique

def lire_donnees_serie():
    # Fonction génératrice pour lire les données série
    while True:
        line = ser.readline().decode('latin-1').strip()
        if line:
            yield line

try:
    # On définit une liste de couleurs à utiliser pour les différentes courbes
    colors = ["blue", "red", "green", "orange", "purple"]
    # On initialise un compteur pour parcourir la liste des couleurs
    color_index = 0

    # Dictionnaires pour stocker les références aux courbes
    lines_temp = {}
    lines_humid = {}

    for data in lire_donnees_serie():
        print(data)

        if data.startswith('3'):
            # Extrait la température et l'humidité des données
            temperature, humidity = int(data[3:5]), int(data[5:])
            print("Temperature : ", temperature, " Humidite : ", humidity)

            try:
                # Extrait l'ID du nœud à partir des données
                node_id = int(data[1])
                print("ID de nœud à utiliser : ", node_id)

                if node_id not in data_by_node:
                    # Crée un dictionnaire pour stocker les données du nœud
                    data_by_node[node_id] = {'temps': [], 'temp': [], 'humid': []}

                    # Utilise la couleur correspondant à l'index courant
                    color = colors[color_index]
                    # Incrémente l'index de couleur modulo la longueur de la liste
                    color_index = (color_index + 1) % len(colors)

                    # Trace les courbes de température et d'humidité avec la couleur choisie
                    # Enregistre les références aux courbes dans les dictionnaires
                    lines_temp[node_id], = ax_temp.plot(data_by_node[node_id]['temps'], data_by_node[node_id]['temp'], color=color, label=f'Noeud {node_id}')
                    lines_humid[node_id], = ax_humid.plot(data_by_node[node_id]['temps'], data_by_node[node_id]['humid'], color=color, label=f'Noeud {node_id}')

                    # Affiche la légende pour chaque axe
                    ax_temp.legend(loc='upper left')
                    ax_humid.legend(loc='upper left')

                # Ajoute les données du nœud au dictionnaire
                data_by_node[node_id]['temps'].append(len(data_by_node[node_id]['temps']) + 1)
                data_by_node[node_id]['temp'].append(temperature)
                data_by_node[node_id]['humid'].append(humidity)

                # Met à jour les données des courbes
                # Utilise la clé node_id pour accéder aux courbes correspondantes
                lines_temp[node_id].set_data(data_by_node[node_id]['temps'], data_by_node[node_id]['temp'])
                lines_humid[node_id].set_data(data_by_node[node_id]['temps'], data_by_node[node_id]['humid'])

                # Ajuste les limites des axes
                ax_temp.relim()
                ax_temp.autoscale_view()
                ax_humid.relim()
                ax_humid.autoscale_view()

                # Rafraîchit le graphique
                plt.draw()
                plt.pause(0.8)  # Ajustez ce délai selon vos besoins

            except ValueError as e:
                print(f"Erreur de conversion de l'ID de nœud en entier : {e}")
            except IndexError as e:
                print(f"Erreur d'index lors de l'extraction de l'ID de nœud : {e}")

except KeyboardInterrupt:
    ser.close()
    print("Connexion série fermée.")
finally:
    ser.close()

plt.ioff()
plt.show()
