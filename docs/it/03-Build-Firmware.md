# Compilazione del firmware dal codice sorgente

## Panoramica

Questo documento descrive come compilare il firmware pixl.js dal codice sorgente. La compilazione produce firmware per due varianti hardware: **OLED** (SH1106 1.3", predefinita) e **LCD** (ST7735 1.8").

Componenti del firmware:

- **Applicazione** — firmware principale con tutte le app (Amiibo, AmiiboDB, AmiiboLink, Chameleon, Player, Giochi, Impostazioni)
- **Bootloader** — bootloader BLE DFU sicuro per aggiornamenti OTA
- **SoftDevice** — stack BLE Nordic S112 v7.2.0 (binario fornito dall'SDK nRF5)

La compilazione è orchestrata da `fw/Makefile` e utilizza GCC + GNU Make.

## Prerequisiti

| Componente | Versione | Necessario? | Fonte |
|---|---|---|---|
| Toolchain ARM GCC | **15.3.rel1** | Sì | [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) |
| nRF5 SDK | **17.1.0_ddde560** | Sì | [Nordic](https://www.nordicsemi.com/Software-and-tools/Software/nRF5-SDK) (registrazione gratuita) |
| nrf-command-line-tools | **10.24.2** | Per flashing/OTA | [Nordic](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools) |
| nrfutil | ultima versione | Per OTA | [Portale Nordic](https://files.nordicsemi.com/) |
| Python 3 | 3.8+ | Per generazione codice | [python.org](https://python.org) |
| GNU Make | qualsiasi | Sì | Preinstallato sulla maggior parte delle distribuzioni Linux |
| Git | qualsiasi | Sì | [git-scm.com](https://git-scm.com) |

## Variabili d'ambiente

Il sistema di compilazione legge queste variabili d'ambiente. Esegui `source fw/env.sh` per impostarle automaticamente.

| Variabile | Necessaria? | Valore tipico | Scopo |
|---|---|---|---|
| `NRF52_SDK_ROOT` | **Sì** | `$HOME/tools/nRF5_SDK_17.1.0_ddde560` | Radice dell'SDK nRF5. Entrambi i Makefile la usano come `$(SDK_ROOT)`. |
| `GNU_INSTALL_ROOT` | **Sì** | `$HOME/tools/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi/bin/` | Percorso dei binari ARM GCC (con `/` finale). Il `Makefile.common` dell'SDK risolve `$(GNU_INSTALL_ROOT)arm-none-eabi-gcc` da questa variabile. |
| `NRF_COMMAND_LINE_TOOLS_ROOT` | Per flashing | `/opt/nrf-command-line-tools` | Percorso di nrf-command-line-tools. Aggiunge `nrfjprog` e `mergehex` al `PATH`. |
| `PATH` (esteso) | **Sì** | Includere `$GNU_INSTALL_ROOT` e `$NRF_COMMAND_LINE_TOOLS_ROOT/bin` | Necessario per il compilatore e gli strumenti Nordic. |

## Metodo 1: Docker (Consigliato)

L'immagine Docker `solosky/nrf52-sdk:latest` (costruita da `fw/docker/Dockerfile`) contiene tutte le dipendenze preinstallate.

```bash
# Creare il container
docker run -it --rm solosky/nrf52-sdk:latest

# Clonare il repository
git clone https://github.com/solosky/pixl.js
cd pixl.js

# Inizializzare i sottomoduli
git submodule update --init --recursive

# Compilare per OLED (consigliato)
cd fw && make all BOARD=OLED RELEASE=1

# Oppure compilare per LCD
cd fw && make all BOARD=LCD RELEASE=1
```

## Metodo 2: Compilazione nativa (Linux)

### 1. Installare il toolchain ARM GCC

```bash
# Scaricare (modificare l'URL se necessario)
wget https://developer.arm.com/-/media/Files/downloads/gnu/15.3.rel1/binrel/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi.tar.xz
sudo tar -xf arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi.tar.xz -C /opt

# Verificare
/opt/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc --version
```

### 2. Installare l'SDK nRF5

Scaricare `nRF5_SDK_17.1.0_ddde560.zip` dal sito Nordic (richiede registrazione gratuita), quindi:

```bash
unzip nRF5_SDK_17.1.0_ddde560.zip -d $HOME/tools/
```

L'SDK include una libreria micro-ecc precompilata. Non è necessario alcun passaggio aggiuntivo.

### 3. Installare gli strumenti opzionali

```bash
# nrf-command-line-tools (per flashing e OTA)
# Scaricare il .deb da Nordic e installare:
sudo dpkg -i nrf-command-line-tools_10.24.2_amd64.deb

# nrfutil (per generazione pacchetti OTA)
# Scaricare dal portale Nordic e posizionare nel PATH
```

### 4. Configurare l'ambiente e compilare

```bash
# Caricare lo script di ambiente
source fw/env.sh

# Se env.sh segnala dipendenze mancanti, installarle prima

# Clonare e compilare
git clone https://github.com/solosky/pixl.js
cd pixl.js
git submodule update --init --recursive
cd fw

# Compilare per OLED
make all BOARD=OLED RELEASE=1

# Oppure compilare per LCD
make all BOARD=LCD RELEASE=1
```

## Generazione del codice

Alcuni file sorgente sono generati automaticamente dagli script Python in `fw/scripts/`. Dopo aver modificato una fonte dati, esegui `make gen` per rigenerare:

| Script | Genera | Fonte dati |
|---|---|---|
| `amiibo_db_gen.py` | `amiidb/db_amiibo.c`, `db_game.c`, `db_link.c` | `fw/data/amiidb_*.csv` |
| `i18n_gen.py` | `i18n/*.c`, `i18n/string_id.h` | `fw/data/i18n.csv` |
| `font_data_gen.py` | file C dei font u8g2 | `fw/data/*.bdf` |
| `resource_gen.py` | Icone delle app | `fw/resources/` |

Dipendenze Python: `pip install -r fw/scripts/requirements.txt`

**Non modificare direttamente i file generati.** Modifica le fonti dati e ri-esegui `make gen`.

## Varianti hardware

| `BOARD=` | Display | Predefinito in | Note |
|---|---|---|---|
| `OLED` | SH1106 OLED 1.3" | `fw/application/Makefile` | Consigliato per la maggior parte degli utenti |
| `LCD` | ST7735 LCD 1.8" | `fw/bootloader/Makefile` | Hardware compatibile con Espruino originale |

**`BOARD` è una costante di compilazione** (`-DBOARD_$(BOARD)`), non una scelta runtime. Il bootloader e l'applicazione devono essere compilati per la stessa variante. Non mescolare un'applicazione LCD con un bootloader OLED.

## Output della compilazione

Tutti gli output si trovano in `fw/_build/`:

| File | Contenuto | Utilizzo |
|---|---|---|
| `pixljs.hex` | Solo applicazione | Aggiornamenti parziali, debug |
| `bootloader.hex` | Bootloader BLE DFU sicuro | Flashing del bootloader |
| `pixljs_all.hex` | SoftDevice + Bootloader + App | Primo flashing via cavo |
| `pixljs_ota_v*.zip` | Pacchetto OTA DFU | Aggiornamenti wireless |
| `pixljs.out` | ELF con simboli di debug | Debug |
| `fw_readme.txt` | Note di rilascio (cinese) | Distribuzione |
| `fw_update.bat` | Script di flashing per Windows | Flashing via cavo |

## Obiettivi Make

| Obiettivo | Descrizione |
|---|---|
| `make all` | Compila bootloader + applicazione + pacchetto OTA |
| `make bl` | Compila solo il bootloader |
| `make app` | Compila solo l'applicazione |
| `make ota` | Genera pacchetto OTA DFU da hex esistente |
| `make full` | Unisce softdevice + bootloader + app + impostazioni in `pixljs_all.hex` |
| `make version` | Rigenera `version.inc.h` da git |
| `make gen` | Rigenera tutti i sorgenti generati (database amiibo, i18n, font, icone) |
| `make flash_ocd` | Compila + flasha applicazione via OpenOCD |
| `make flash_all_ocd` | Compila + flasha immagine completa via OpenOCD |
| `make clean` | Rimuove gli artefatti di compilazione |
| `make privgen` | Genera coppia di chiavi per firma DFU |

## Risoluzione dei problemi

| Sintomo | Causa probabile |
|---|---|
| `arm-none-eabi-gcc: command not found` | `GNU_INSTALL_ROOT` non impostato o non in `PATH` |
| `fatal error: sdk_config.h: No such file or directory` | `NRF52_SDK_ROOT` non impostato o versione SDK errata |
| `chameleon-ultra/ ... No such file or directory` | Dimenticato `git submodule update --init --recursive` |
| `make: nrfjprog: Command not found` | nrf-command-line-tools non installato o non in `PATH` |
| `mergehex: command not found` | nrf-command-line-tools non installato |
| Schermo alterato o vuoto dopo il flashing | Variante `BOARD` errata (LCD vs OLED) |
| `region FLASH overflowed` | Applicazione troppo grande per la flash disponibile |
