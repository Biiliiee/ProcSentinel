// clang-format off
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <wininet.h>
#pragma comment(lib, "wininet.lib")
// clang-format on

void EnviarWebhook(const char *mensagem) {
  HINTERNET hInternet =
      InternetOpen("app", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

  HINTERNET hConnect =
      InternetConnect(hInternet, "discord.com", INTERNET_DEFAULT_HTTPS_PORT,
                      NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

  HINTERNET hRequest = HttpOpenRequest(
      hConnect, "POST",
      "/api/webhooks/SEU_ID/SEU_TOKEN",
      NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);

  const char *headers = "Content-Type: application/json";

  char json[512];
  sprintf(json, "{\"content\": \"%s\"}", mensagem);

  HttpSendRequest(hRequest, headers, strlen(headers), json, strlen(json));

  InternetCloseHandle(hRequest);
  InternetCloseHandle(hConnect);
  InternetCloseHandle(hInternet);
}

typedef struct {
  DWORD PID;
  DWORD PPID;
  DWORD Threads;
  char Nome[MAX_PATH];
} ListaPID;

int ProcessoExiste(ListaPID *minhaLista, DWORD pid, int contador) {
  for (int i = 0; i < contador; i++) {
    if (minhaLista[i].PID == pid) {
      return 1;
    }
  }
  return 0;
}

void EnumerarProcessos(PROCESSENTRY32 *pe, HANDLE snapshot, int *contador,
                       ListaPID *minhaLista) {
  if (Process32First(snapshot, pe)) {
    do {
      minhaLista[*contador].PID = pe->th32ProcessID;
      minhaLista[*contador].PPID = pe->th32ParentProcessID;
      minhaLista[*contador].Threads = pe->cntThreads;
      strncpy(minhaLista[*contador].Nome, pe->szExeFile, MAX_PATH - 1);
      minhaLista[*contador].Nome[MAX_PATH - 1] = '\0';
      (*contador)++;
    } while (Process32Next(snapshot, pe));
  }
}

int main() {
  PROCESSENTRY32 pe;
  HANDLE snapshot;
  int contador = 0;

  ListaPID MinhaLista[1024];
  ListaPID ListaNova[1024];

  pe.dwSize = sizeof(PROCESSENTRY32);

  snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (snapshot == INVALID_HANDLE_VALUE) {
    printf("Falha ao criar o snapshot. Erro: %lu\n", GetLastError());
    return 1;
  }

  printf("Snapshot criado com sucesso.\n");

  EnumerarProcessos(&pe, snapshot, &contador, MinhaLista);
  CloseHandle(snapshot);

  for (int i = 0; i < contador; i++) {
    printf("Nome: %s \n PID: %lu \n", MinhaLista[i].Nome, MinhaLista[i].PID);
  }

  while (1) {
    int contadorNovo = 0;

    HANDLE snapshotNovo = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshotNovo != INVALID_HANDLE_VALUE) {
      EnumerarProcessos(&pe, snapshotNovo, &contadorNovo, ListaNova);

      for (int i = 0; i < contadorNovo; i++) {
        DWORD pid = ListaNova[i].PID;

        if (!ProcessoExiste(MinhaLista, pid, contador)) {
          printf(
              "\nNOVO PROCESSO \n \n Nome: %s \n PID: %lu \n PID Pai: %lu \n "
              "Threads: %lu\n",
              ListaNova[i].Nome, pid, ListaNova[i].PPID, ListaNova[i].Threads);

          if (stricmp(ListaNova[i].Nome, "powershell.exe") == 0) {
            printf("PowerShell aberto!\n");

            char mensagemAlerta[512];

            snprintf(mensagemAlerta, sizeof(mensagemAlerta),
                     "⚠️ **ALERTA DE SEGURANÇA**\\n\\n"
                     " **Nome:** %s\\n"
                     " **PID:** %lu\\n"
                     " **PID Pai (PPID):** %lu\\n"
                     " **Threads:** %lu",
                     ListaNova[i].Nome, pid, ListaNova[i].PPID,
                     ListaNova[i].Threads);

            EnviarWebhook(mensagemAlerta);
          }
        }
      }
    }

    memcpy(MinhaLista, ListaNova, sizeof(ListaPID) * contadorNovo);
    contador = contadorNovo;
    CloseHandle(snapshotNovo);
    Sleep(1000);
  }

  return 0;
}
