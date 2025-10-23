# 🧠 GUIDA INTELLIGENTE alla Risoluzione degli Esercizi su iptables

---

## 🗺️ 1. Capisci bene lo scenario

Prima di scrivere comandi, **analizza la topologia** e **l’obiettivo**.

Cerca nel testo:

| Info                 | Esempio                            | Significato                 |
| -------------------- | ---------------------------------- | --------------------------- |
| Interfaccia pubblica | `eth0` con `198.51.100.5`          | Verso Internet              |
| Interfaccia privata  | `eth1` con `10.10.20.1`            | Verso LAN interna           |
| Rete privata         | `10.10.20.0/24`                    | Host interni                |
| Gateway predefinito  | IP privato del firewall            | Tutti i client lo usano     |
| Server interno       | `10.10.20.50`                      | Da pubblicare con DNAT      |
| Servizio             | `tcp/80`, `tcp/990`, `udp/53` ecc. | Porta/Protocollo da gestire |

💡 **Visualizza mentalmente lo schema:**

```
[Internet] --- eth0 [Firewall] eth1 --- [LAN 10.10.20.0/24]
```

---

## 🧩 2. Inizia sempre da Pulizia e Policy di default

Quasi tutti gli esercizi iniziano con:

> “Elimina tutte le regole esistenti” e “Scarta tutto a meno che non sia esplicitamente permesso”.

Scrivi subito:

```bash
-F                   # Pulisce la tabella filter
-t nat -F            # Pulisce la tabella nat

-P INPUT DROP
-P FORWARD DROP
```

✅ **Perché:** parti sempre da una base sicura: tutto è bloccato finché non autorizzi tu.

⚠️ *Non impostare DROP sulla catena OUTPUT a meno che non sia richiesto esplicitamente!*
Altrimenti il firewall non riuscirà più a risolvere nomi o scaricare aggiornamenti.

---

## 🧱 3. Gestisci le regole INPUT (verso il firewall)

Queste regole controllano cosa **arriva al firewall stesso**, non cosa passa attraverso.

| Descrizione           | Comando tipico                                                                 | Note                          |
| --------------------- | ------------------------------------------------------------------------------ | ----------------------------- |
| ICMP (ping)           | `-A INPUT -i eth0 -p icmp -j ACCEPT` <br> `-A INPUT -i eth1 -p icmp -j ACCEPT` | Spesso consentito su entrambe |
| SSH da LAN            | `-A INPUT -i eth1 -p tcp --dport 22 -j ACCEPT`                                 | Accesso amministrativo locale |
| SSH da host specifico | `-A INPUT -i eth1 -s 192.168.30.200 -p tcp --dport 22 -j ACCEPT`               | Solo da IP autorizzato        |

💡 **Ricorda:** INPUT = traffico *destinato al firewall*, non ai server interni.

---

## 🚦 4. Regole FORWARD (tra interfacce)

Le catene `FORWARD` gestiscono il traffico **che passa attraverso** il firewall.

| Caso                                 | Significato                                               | Esempio                                                               |
| ------------------------------------ | --------------------------------------------------------- | --------------------------------------------------------------------- |
| LAN → Internet                       | “Consenti pacchetti ricevuti su eth1 e in uscita su eth0” | `-A FORWARD -i eth1 -o eth0 -j ACCEPT`                                |
| Traffico di risposta                 | “Consenti pacchetti con stato ESTABLISHED,RELATED”        | `-A FORWARD -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT`     |
| DNAT (traffico verso server interno) | “Assicurati che possano raggiungere quell’host”           | `-A FORWARD -i eth0 -o eth1 -p tcp -d <IP> --dport <porta> -j ACCEPT` |

💡 **Regola d’oro:** ogni DNAT richiede anche un corrispondente `FORWARD ACCEPT`.

---

## 🌐 5. Regole NAT: POSTROUTING e PREROUTING

Le regole NAT modificano **gli indirizzi IP dei pacchetti**.

---

### 🔁 POSTROUTING (SNAT / MASQUERADE)

Usata per far sì che gli host interni ricevano risposte da Internet.

| Se l’IP pubblico è dinamico                   | Se l’IP pubblico è fisso                                          |
| --------------------------------------------- | ----------------------------------------------------------------- |
| `-t nat -A POSTROUTING -o eth0 -j MASQUERADE` | `-t nat -A POSTROUTING -o eth0 -j SNAT --to-source <IP_pubblico>` |

💡 `MASQUERADE` è una forma automatica di SNAT, perfetta quando il firewall prende l’IP da DHCP.

---

### 🎯 PREROUTING (DNAT)

Serve per pubblicare un servizio interno all’esterno.

```bash
-t nat -A PREROUTING -i eth0 -p tcp --dport <porta_esterna> \
    -j DNAT --to-destination <IP_interno>:<porta_interna>

-A FORWARD -i eth0 -o eth1 -p tcp -d <IP_interno> --dport <porta_interna> -j ACCEPT
```

💡 Esempi tipici:

| Servizio | Porta esterna | Destinazione interna |
| -------- | ------------- | -------------------- |
| HTTP     | 80            | `10.10.x.x:8080`     |
| FTPS     | 990           | `10.10.x.x:19990`    |
| DNS      | 53            | `172.16.x.x:53`      |

---

## 🔍 6. Regola magica: conntrack

Quasi sempre serve:

```bash
-A FORWARD -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
```

💡 Permette ai pacchetti di risposta di tornare indietro.
Senza questa, la connessione funziona solo in una direzione!

---

## 🧠 7. Come tradurre ogni riga della tabella

Allenati a “vedere” le frasi e convertirle mentalmente in regole:

| Testo nella tabella                         | Traduzione iptables                                                                         |
| ------------------------------------------- | ------------------------------------------------------------------------------------------- |
| “Elimina tutte le regole esistenti”         | `-F` e `-t nat -F`                                                                          |
| “Scarta tutto a meno che non sia permesso”  | `-P INPUT DROP`, `-P FORWARD DROP`                                                          |
| “Consenti ICMP”                             | `-A INPUT -i <iface> -p icmp -j ACCEPT`                                                     |
| “Consenti SSH (tcp/22)”                     | `-A INPUT -i <iface> -p tcp --dport 22 -j ACCEPT`                                           |
| “Consenti traffico da eth1 a eth0”          | `-A FORWARD -i eth1 -o eth0 -j ACCEPT`                                                      |
| “Consenti connessioni stabilite”            | `-A FORWARD -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT`                           |
| “MASQUERADE pacchetti in uscita”            | `-t nat -A POSTROUTING -o eth0 -j MASQUERADE`                                               |
| “SNAT all’IP pubblico”                      | `-t nat -A POSTROUTING -o eth0 -j SNAT --to-source <IP>`                                    |
| “Applica DNAT …”                            | `-t nat -A PREROUTING -i eth0 -p tcp --dport <porta> -j DNAT --to-destination <IP>:<porta>` |
| “Assicurati che possano raggiungere l’host” | `-A FORWARD -i eth0 -o eth1 -p tcp -d <IP> --dport <porta> -j ACCEPT`                       |

---

## ⚠️ 8. Errori comuni all’esame

| Errore                    | Causa                                       | Soluzione                                   |
| ------------------------- | ------------------------------------------- | ------------------------------------------- |
| Dimenticare `-t nat`      | Regola finisce nella tabella sbagliata      | Sempre specificare `-t nat`                 |
| DNAT senza FORWARD        | Il pacchetto viene DNATtato ma poi bloccato | Aggiungi regola FORWARD corrispondente      |
| Mancanza conntrack        | Nessuna risposta alle connessioni           | Aggiungi `--ctstate ESTABLISHED,RELATED`    |
| Interfaccia sbagliata     | Direzione invertita                         | Controlla bene `-i` (input) e `-o` (output) |
| MASQUERADE con IP statico | Funziona ma non è efficiente                | Usa SNAT se IP è fisso                      |

---

## 🧩 9. Come verificare le regole

Dopo averle impostate:

```bash
iptables -L -v -n
iptables -t nat -L -v -n
```

Test pratici:

* Da LAN: `ping 8.8.8.8` → verifica MASQUERADE/SNAT
* Da Internet (o simulato): `telnet <IP_pubblico> <porta>` → verifica DNAT
* Visualizza connessioni: `cat /proc/net/nf_conntrack`

---

## 🧠 10. Regole mentali da memorizzare

| Catena          | Significato                                 |
| --------------- | ------------------------------------------- |
| **INPUT**       | Pacchetti diretti al firewall               |
| **FORWARD**     | Pacchetti che attraversano il firewall      |
| **OUTPUT**      | Pacchetti generati dal firewall             |
| **PREROUTING**  | DNAT (modifica della destinazione)          |
| **POSTROUTING** | SNAT / MASQUERADE (modifica della sorgente) |
| **eth0**        | Verso Internet                              |
| **eth1**        | Verso LAN                                   |

💡 Con questa logica, capisci subito la direzione di ogni pacchetto.

---

## 💎 11. Template universale da riutilizzare

Puoi adattarlo a qualsiasi esercizio:

```bash
# Pulizia
-F
-t nat -F

# Policy di default
-P INPUT DROP
-P FORWARD DROP

# INPUT
-A INPUT -i eth0 -p icmp -j ACCEPT
-A INPUT -i eth1 -p icmp -j ACCEPT
-A INPUT -i eth1 -p tcp --dport 22 -j ACCEPT

# FORWARD
-A FORWARD -i eth1 -o eth0 -j ACCEPT
-A FORWARD -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT

# NAT
-t nat -A POSTROUTING -o eth0 -j MASQUERADE

# (DNAT se richiesto)
-t nat -A PREROUTING -i eth0 -p tcp --dport <porta_ext> -j DNAT --to-destination <IP_int>:<porta_int>
-A FORWARD -i eth0 -o eth1 -p tcp -d <IP_int> --dport <porta_int> -j ACCEPT
```

---
