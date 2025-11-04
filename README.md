# WebSocket-Touch

WebSocket avec module de contrôle équipé d'une interface locale TFT Touch.
Mise à jour en temps réel de la page Html et de l'écran TFT.

https://github.com/user-attachments/assets/20481370-a162-48de-956b-a4bc53e7d775

Le module est configuré à la base pour la commande de trois équipements, mais peut être adapté à un nombre variable d'équipements.

![WebSocket_Esp-Now](https://github.com/user-attachments/assets/4f6566d8-df2a-41cd-ab7f-fe667fe69716)

Evolution du projet par l'adjonction d'un ESP8266 connecté aux sorties relais de l'ESP32 pour envoyer les commandes à d'autres ESP8266 placés à distance en utilisant le protocole Esp-Now.

Secusisation de l'état des récepteurs.
Lors d'un redemmarage d'un récepteur l'envoi d'une requète à l'émetteur pour un revoi de l'état des équipements,
car l'état peut avoir changé durant la mise hors circuit d'un récepteur.
Pour des raisons de simplification le renvoi de l'état se fait pour les trois équipements.


Dernière évolution, création d'un module ESP32 avec 3 leds et 3 boutons poussoirs pour servire de télécommande au WebSocket.

