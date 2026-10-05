# 🛡️ ProcSentinel — Real-Time Windows Process Monitor & Discord Alert System

![Language](https://img.shields.io/badge/Language-C-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6.svg)
![API](https://img.shields.io/badge/API-Win32%20%2F%20WinINet-00599C.svg)
![Category](https://img.shields.io/badge/Category-System%20Security%20%2F%20Monitoring-red.svg)

> **ProcSentinel** é uma ferramenta de monitoramento de processos em tempo real desenvolvida puramente em **C nativo (Win32 API)**. O sistema captura *snapshots* periódicos da memória do Windows, compara o estado dos processos e dispara alertas instantâneos via **Discord Webhook** sempre que processos específicos (como `powershell.exe` ou `cmd.exe`) são iniciados.

---

## Arquitetura e Fluxo de Funcionamento

O diagrama abaixo ilustra o ciclo de vida e a lógica de verificação de processos realizada pelo **ProcSentinel**:

<p align="center">
  <img src="assets/Fluxo de Monitoramento de Processos.png" alt="Fluxo de Monitoramento de Processos" width="100%" />
</p>

###  Passos da Execução:

1. **Snapshot Inicial (`MinhaLista`)**: 
   - Ao iniciar, o programa utiliza `CreateToolhelp32Snapshot` para capturar todos os PIDs ativos e registrar o estado inicial do sistema no vetor `MinhaLista`.
2. **Loop de Varredura (`ListaNova`)**: 
   - A cada 1 segundo (`Sleep(1000)`), uma nova "foto" do sistema é tirada e armazenada no vetor `ListaNova`.
3. **Comparação de PIDs (`ProcessoExiste`)**: 
   - O sistema percorre cada processo da `ListaNova` e verifica se ele já existia na `MinhaLista`. Se o PID não for encontrado, significa que um **novo processo foi aberto**.
4. **Gatilho & Alerta Webhook**: 
   - Se o novo processo for um executável monitorado (ex: `powershell.exe`), o programa formata uma payload HTTP em JSON e faz a requisição via `WinINet` para o Discord.

---



## Como Compilar e Executar

### Pré-requisitos
- Compilador **GCC (MinGW)** no Windows.

### Compilação via Terminal:

Você pode compilar o projeto usando a flag `-lwininet` para linkar a biblioteca de rede:

```bash
# Compilar com GCC 64-bits
gcc ProcSentinel.c -lwininet -o ProcSentinel.exe

# Executar a aplicação
.\ProcSentinel.exe
```

Ou utilizar o script auxiliar incluso no repositório:

```cmd
run64.bat ProcSentinel.c
```

---

## ⚙️ Configuração do Webhook

No arquivo `ProcSentinel.c`, configure a constante da sua URL do Discord Webhook na função `EnviarWebhook`:

```c
HINTERNET hRequest = HttpOpenRequest(
    hConnect, 
    "POST",
    "/api/webhooks/SEU_ID/SEU_TOKEN", // <-- Insira o ID e Token do seu Discord aqui
    NULL, NULL, NULL, 
    INTERNET_FLAG_SECURE, 0
);
```

---

## 📌 Exemplo de Notificação Recebida no Discord

```text
⚠️ ALERTA DE SEGURANÇA

 Nome: powershell.exe
 PID: 10660
 PID Pai (PPID): 12656
 Threads: 21
```

---


Este projeto foi desenvolvido exclusivamente para fins de **pesquisa de arquitetura interna do Windows e desenvolvimento de ferramentas defensivas (Blue Team / Purple Team)**.
