# TimeLine ESP8266 Controller

Controlador de espetáculo, escrito em **C++ / [openFrameworks](https://openframeworks.cc) (v0.11.2)**, que sincroniza uma **timeline** (trilha de vídeo + faixas de _switches_) com **luzes/relés acionados por dispositivos ESP8266** espalhados pela cena — tipicamente um por bailarino/performer.

Cada faixa da timeline liga e desliga um relé de um ESP8266 via rede, em sincronia com um vídeo de fundo (o "telão"). Um disparo externo via **OSC** inicia o show.

> **Origem do projeto:** desenvolvido por Felipe Maion em 2022 para controlar a iluminação de figurinos/cenário de uma apresentação de dança. O nome do diretório original era `ESP8266_timeline_controller`. Este repositório consolida e documenta esse projeto.

---

## Sumário

- [Arquitetura](#arquitetura)
- [Como funciona a timeline](#como-funciona-a-timeline)
- [Protocolo de comunicação com os ESP8266](#protocolo-de-comunicação-com-os-esp8266)
- [Como o firmware do ESP8266 deveria estar configurado](#como-o-firmware-do-esp8266-deveria-estar-configurado)
- [Controle remoto via OSC (start do show)](#controle-remoto-via-osc-start-do-show)
- [Configuração dos dispositivos (IPs)](#configuração-dos-dispositivos-ips)
- [Dependências (addons)](#dependências-addons)
- [Como compilar e rodar](#como-compilar-e-rodar)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Estado do código e limitações conhecidas](#estado-do-código-e-limitações-conhecidas)
- [Roadmap / melhorias sugeridas](#roadmap--melhorias-sugeridas)

---

## Arquitetura

```
                       ┌─────────────────────────────────────────────┐
   OSC /mr_start  ───► │   Controlador openFrameworks (Mac/PC)        │
   (porta 8888)        │                                             │
                       │   ┌───────────────┐   ┌──────────────────┐  │
                       │   │  ofxTimeline  │   │  ofxPanel (GUI)  │  │
                       │   │  + vídeo      │   │  device / play   │  │
                       │   └───────┬───────┘   └──────────────────┘  │
                       │           │ switches on/off por faixa        │
                       │     ┌─────▼──────┐                           │
                       │     │  Dancer[]  │  (1 por performer)        │
                       │     └─────┬──────┘                           │
                       └───────────┼───────────────────────────────-─┘
                                   │  OSC /relay0 /relay1  (UDP 5555)
              ┌────────────────────┼────────────────────┬───────────────┐
              ▼                    ▼                    ▼               ▼
        ┌───────────┐        ┌───────────┐        ┌───────────┐   ┌───────────┐
        │  ESP8266  │        │  ESP8266  │        │  ESP8266  │   │  ESP8266  │
        │  Márcio   │        │  Tuanny   │        │  Amanda   │   │  Karyne   │
        │ relay0/1  │        │ relay0    │        │ relay0    │   │ relay0    │
        └───────────┘        └───────────┘        └───────────┘   └───────────┘
```

- **Controlador (este projeto):** aplicação openFrameworks. Mantém a timeline, reproduz o vídeo e, a cada frame, lê o estado de cada faixa de _switch_ e envia os comandos de relé para o ESP8266 correspondente.
- **Dispositivos (ESP8266):** cada um possui um IP fixo na rede local e um pequeno servidor que recebe os comandos e aciona um ou dois relés (luzes do figurino/cenário).
- **`Dancer`** (`src/myDancer.{h,cpp}`): abstrai um performer + seu ESP8266. Conhece `ip`, `name`, `port` (5555) e expõe `relay0_ON/OFF`, `relay1_ON/OFF`. É o caminho **ativo** de comunicação (OSC/UDP).
- **`Device`** (`src/myDevice.{h,cpp}`): abstração mais nova baseada em **TCP** (`ofxTCPClient`), integrada à GUI com `ofParameter`. Estava em desenvolvimento como evolução do `Dancer` (ver [protocolo](#protocolo-de-comunicação-com-os-esp8266)).

---

## Como funciona a timeline

O coração do app está em `src/ofApp.cpp`:

1. No `setup()`, uma `ofxTimeline` recebe uma **trilha de vídeo** (`bin/data/telao.mov`). O vídeo controla o tempo (`setTimecontrolTrack`), então o playback do vídeo "arrasta" toda a timeline.
2. Para cada `Dancer` é criada uma **faixa de switches** (`timeline.addSwitches(dancer.name)`). O performer **Márcio** tem uma faixa extra, `"Márcio-CORAÇÃO"`, que controla o `relay1` (um segundo efeito — o "coração").
3. A cada frame, no `draw()`:
   - `timeline.isSwitchOn(dancer.name)` → liga/desliga o `relay0` daquele dispositivo.
   - `timeline.isSwitchOn("Márcio-CORAÇÃO")` → liga/desliga o `relay1` do Márcio.
   - Os envios são feitos de forma **assíncrona** (`std::async`) para não travar o loop de render enquanto a rede responde.
4. Faixas auxiliares: `flags`, faixa de cor (`addColors("COR")`).

> As faixas/keyframes ficam serializadas em arquivos XML na raiz do projeto (`timeline0_*.xml`) e em `bin/data/`. Há um problema conhecido de salvar/restaurar a timeline — por isso as faixas são recriadas "na mão" no `setup()`.

---

## Protocolo de comunicação com os ESP8266

O projeto passou por **três gerações de protocolo**. Conhecer as três ajuda a entender o firmware esperado e a portar o controlador.

### 1. HTTP (legado — "OLD WORKING VERSION") — porta 80

Primeira versão, ainda presente comentada em `Dancer::connect()`. O ESP8266 expunha um servidor HTTP e o controlador fazia `GET`:

| Requisição                | Efeito              |
|---------------------------|---------------------|
| `GET http://<ip>/4/on`    | liga GPIO4 (relay0) |
| `GET http://<ip>/4/off`   | desliga GPIO4       |
| `GET http://<ip>/5/on`    | liga GPIO5 (relay1) |
| `GET http://<ip>/5/off`   | desliga GPIO5       |

Resposta `200 OK` = sucesso. Os números `4`/`5` correspondem aos GPIOs do ESP8266.

### 2. OSC sobre UDP — porta 5555 **(caminho ativo hoje)**

Usado por `Dancer` (`src/myDancer.cpp`). Para cada dispositivo existe um `ofxOscSender` apontando para `<ip>:5555`. Os comandos são mensagens OSC:

| Endereço OSC | Argumento (int) | Efeito          |
|--------------|-----------------|-----------------|
| `/relay0`    | `1`             | liga o relé 0   |
| `/relay0`    | `0`             | desliga o relé 0|
| `/relay1`    | `1`             | liga o relé 1   |
| `/relay1`    | `0`             | desliga o relé 1|

### 3. TCP (em desenvolvimento) — porta 585

Usado por `Device` (`src/myDevice.cpp`) via `ofxTCPClient`. Comandos são **caracteres únicos** enviados pela conexão TCP:

| Caractere | Efeito           |
|-----------|------------------|
| `"0"`     | relay0 OFF       |
| `"1"`     | relay0 ON        |
| `"2"`     | relay1 OFF       |
| `"3"`     | relay1 ON        |

Ao conectar, o controlador envia `"0"` e `"2"` para garantir tudo desligado.

> **Recomendação:** padronizar em **um** protocolo. O OSC/UDP (gen. 2) é o mais simples e o que está em produção; o TCP (gen. 3) dá confirmação de conexão. Veja o [roadmap](#roadmap--melhorias-sugeridas).

---

## Como o firmware do ESP8266 deveria estar configurado

> O código do firmware **não está neste repositório** (pode ter se perdido ou estar em outra pasta da máquina original). A descrição abaixo **reconstrói** o servidor esperado a partir do protocolo usado pelo controlador, para que qualquer ESP8266 possa ser reprogramado e voltar a funcionar.

### Hardware esperado

- Placa **ESP8266** (NodeMCU / Wemos D1 mini ou similar).
- 1 ou 2 **módulos de relé** (ou MOSFETs) acionando as luzes do figurino/cenário.
- Mapeamento de GPIO (compatível com o protocolo HTTP legado `/4` e `/5`):

  | Relé    | GPIO   | Pino NodeMCU |
  |---------|--------|--------------|
  | relay0  | GPIO4  | D2           |
  | relay1  | GPIO5  | D1           |

### Requisitos de rede

- **IP fixo** na LAN (o controlador tem os IPs _hard-coded_ — ver [abaixo](#configuração-dos-dispositivos-ips)). Configure via DHCP reservado no roteador **ou** IP estático no firmware.
- Todos os ESP8266 e o computador controlador na **mesma rede / sub-rede**.
- Porta de escuta **UDP 5555** para OSC (protocolo ativo).

### Esboço de firmware (OSC/UDP — protocolo ativo)

Exemplo mínimo usando Arduino core para ESP8266 + biblioteca [`OSC` (CNMAT)](https://github.com/CNMAT/OSC):

```cpp
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>

// ---- Configuração de rede ----
const char* ssid     = "SUA_REDE";
const char* password = "SUA_SENHA";

// IP fixo deste dispositivo (precisa bater com o IP no controlador)
IPAddress local_IP(192, 168, 15, 14);
IPAddress gateway (192, 168, 15, 1);
IPAddress subnet  (255, 255, 255, 0);

const unsigned int OSC_PORT = 5555;   // porta UDP que o controlador usa

const int RELAY0 = 4;   // GPIO4 / D2
const int RELAY1 = 5;   // GPIO5 / D1

WiFiUDP Udp;

void setRelay(int pin, int value) {
  // Relés costumam ser ativos em LOW; inverta se necessário.
  digitalWrite(pin, value ? HIGH : LOW);
}

void onRelay0(OSCMessage &msg) { setRelay(RELAY0, msg.getInt(0)); }
void onRelay1(OSCMessage &msg) { setRelay(RELAY1, msg.getInt(0)); }

void setup() {
  pinMode(RELAY0, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  setRelay(RELAY0, 0);
  setRelay(RELAY1, 0);

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(250);

  Udp.begin(OSC_PORT);
}

void loop() {
  int size = Udp.parsePacket();
  if (size > 0) {
    OSCMessage msg;
    while (size--) msg.fill(Udp.read());
    if (!msg.hasError()) {
      msg.dispatch("/relay0", onRelay0);
      msg.dispatch("/relay1", onRelay1);
    }
  }
}
```

### Variante: servidor HTTP (compatível com o protocolo legado)

Caso prefira a 1ª geração (mais fácil de testar pelo navegador):

```cpp
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);
const int RELAY0 = 4, RELAY1 = 5;

void handle(int pin, int value) {
  digitalWrite(pin, value ? HIGH : LOW);
  server.send(200, "text/plain", "OK");
}

void setup() {
  pinMode(RELAY0, OUTPUT); pinMode(RELAY1, OUTPUT);
  // ... conectar no WiFi com IP fixo ...
  server.on("/4/on",  []{ handle(RELAY0, 1); });
  server.on("/4/off", []{ handle(RELAY0, 0); });
  server.on("/5/on",  []{ handle(RELAY1, 1); });
  server.on("/5/off", []{ handle(RELAY1, 0); });
  server.begin();
}

void loop() { server.handleClient(); }
```

### Variante: servidor TCP bruto (porta 585)

Para o caminho `Device`/`ofxTCPClient`: abrir um `WiFiServer` na porta **585** e, a cada byte recebido, interpretar `'0'/'1'/'2'/'3'` conforme a [tabela TCP](#3-tcp-em-desenvolvimento--porta-585).

> ⚠️ **Lógica do relé:** muitos módulos de relé são **ativos em LOW** (o relé liga quando o pino vai a `LOW`). Se as luzes ligarem invertidas, troque o sentido em `setRelay()`.

---

## Controle remoto via OSC (start do show)

Além de enviar comandos, o controlador **recebe** OSC para iniciar o espetáculo:

- **Porta de escuta:** `8888` (`#define PORT 8888` em `ofApp.h`).
- **Mensagem:** endereço `/mr_start`, argumento booleano.
- **Efeito:** ao receber `/mr_start true`, o vídeo (e a timeline) é reiniciado do frame 0 e começa a tocar.

Isso permite disparar o show a partir de outro software/console (ex.: QLab, um pedal MIDI→OSC, outro computador).

---

## Configuração dos dispositivos (IPs)

Atualmente os performers e seus IPs estão **fixos no código** (`src/ofApp.h`):

```cpp
Dancer dancer1 = Dancer("192.168.15.14", "Márcio");
Dancer dancer2 = Dancer("192.168.15.17", "Tuanny");
Dancer dancer3 = Dancer("192.168.0.102", "Amanda");
Dancer dancer4 = Dancer("192.168.0.103", "Karyne");
```

Para usar com outros dispositivos, edite esses IPs/nomes (e garanta que cada ESP8266 responda no IP correspondente). O Márcio usa **dois relés** (`relay0` e a faixa extra `Márcio-CORAÇÃO` → `relay1`); os demais usam só `relay0`.

> ℹ️ Note que os dispositivos estão em **duas sub-redes** diferentes (`192.168.15.x` e `192.168.0.x`) — confirme que a rede do show roteia/abrange ambas, ou unifique todos na mesma sub-rede.

---

## Dependências (addons)

De `addons.make`:

```
ofxXmlSettings
ofxControlPanel
ofxGui
ofxMSATimer
ofxNetwork
ofxOsc
ofxPoco
ofxRange
ofxTextInputField
ofxTimecode
ofxTween
ofxTimeline
```

O addon central é o **[ofxTimeline](https://github.com/YCAMInterlab/ofxTimeline)** (com suas dependências: `ofxRange`, `ofxTextInputField`, `ofxTimecode`, `ofxTween`, `ofxMSATimer`). Rede/protocolo: `ofxNetwork` (TCP) e `ofxOsc` (OSC/UDP).

---

## Como compilar e rodar

Projeto de openFrameworks **v0.11.2**. Ele precisa viver dentro de uma instalação do OF, em `OF/apps/myApps/`.

### Pré-requisitos

1. Baixar o **openFrameworks 0.11.2** para a sua plataforma.
2. Clonar este repositório em `OF/apps/myApps/TimeLineESP8266Controller`.
3. Instalar os [addons](#dependências-addons) em `OF/addons/` (alguns não vêm por padrão, como `ofxTimeline`, `ofxControlPanel`, `ofxMSATimer`).
4. Colocar o vídeo de fundo em `bin/data/telao.mov` (não versionado — ver `.gitignore`).

### macOS (Xcode)

- Abrir `ESP8266_timeline_controller.xcodeproj` e compilar (target Release).
- Ou usar o **Project Generator** do OF para regenerar os arquivos de projeto após ajustar `addons.make`.

### Linux / macOS (Makefile)

```bash
make            # compila
make RunRelease # executa
```

> Se os caminhos de addon mudarem, rode o **projectGenerator** do OF apontando para esta pasta para recriar `config.make`/`*.xcodeproj`.

---

## Estrutura do projeto

```
.
├── README.md
├── addons.make            # lista de addons do OF
├── config.make            # configuração de build do OF
├── Makefile
├── Project.xcconfig
├── ESP8266_timeline_controller.xcodeproj/
├── src/
│   ├── main.cpp           # bootstrap (janela 1024x768)
│   ├── ofApp.h / ofApp.cpp# app: timeline, vídeo, GUI, OSC, loop de relés
│   ├── myDancer.h/.cpp    # 1 performer + ESP8266 (OSC/UDP 5555) [ativo]
│   └── myDevice.h/.cpp    # dispositivo via TCP (porta 585) [em desenvolvimento]
├── bin/data/              # assets em runtime (vídeo, settings.xml, GUI) — não versionado
└── timeline0_*.xml        # estado serializado das faixas/keyframes da timeline
```

---

## Estado do código e limitações conhecidas

Este era um projeto pessoal/experimental; vale registrar honestamente o estado:

- **Sem boas práticas consolidadas:** IPs, nomes e faixas estão _hard-coded_; muito código de UI comentado em `ofApp::draw()`.
- **Três protocolos coexistindo** (HTTP legado, OSC ativo, TCP novo) sem uma camada de abstração única.
- **Persistência da timeline instável:** as faixas são recriadas manualmente no `setup()` por causa de um bug de salvar/carregar.
- **Envio de relé a cada frame:** no `draw()`, o estado é reenviado continuamente (não só na borda de mudança), gerando tráfego de rede redundante.
- **Sem tratamento robusto de reconexão** dos dispositivos no caminho OSC (UDP é "fire-and-forget").

---

## Roadmap / melhorias sugeridas

- [ ] **Unificar o protocolo** num só (sugestão: OSC/UDP) e isolar atrás de uma interface comum a `Dancer`/`Device`.
- [ ] Mover **IPs/nomes/faixas para um arquivo de configuração** (`devices.json`/`xml`) em vez de _hard-code_.
- [ ] Enviar comando de relé **apenas na transição** de estado (borda), não a cada frame.
- [ ] **Recuperar/versionar o firmware** do ESP8266 neste mesmo repositório (pasta `firmware/`), usando os esboços acima como base.
- [ ] Heartbeat/ACK dos dispositivos para indicar status de conexão na GUI.
- [ ] Documentar/automatizar a instalação dos addons.

---

## Licença

Distribuído sob a licença **MIT**. Veja o arquivo [`LICENSE`](LICENSE) para o texto completo.
