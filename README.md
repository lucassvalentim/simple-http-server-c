# Projeto Client-Server HTTP

Este projeto implementa uma comunicação cliente-servidor em C utilizando **sockets**, com suporte a requisições HTTP simples.

---

## Estrutura do Repositório

```
client-server/
│
├── client/               # Código-fonte do cliente
├── server/               # Código-fonte do servidor HTTP
├── site/                 # Diretório de arquivos estáticos servidos pelo servidor (HTML, CSS, etc.)
└── makefile              # Arquivo Makefile para compilar o projeto
```

---

## Compilação

Para compilar o projeto, execute o comando abaixo na raiz do repositório:

```bash
make
```

Isso irá compilar tanto o **cliente** quanto o **servidor**.

Se preferir compilar manualmente:

```bash
gcc -o client/client client/client.c
gcc -o server/server server/server.c
```

---

## Execução

### 1. Inicie o servidor HTTP

```bash
./server/main ./site
```

O argumento `./site` é o diretório de onde os arquivos HTML serão servidos.

Por padrão, o servidor escuta na **porta 3000**.

Se existir um arquivo `index.html` dentro do diretório fornecido, ele será retornado ao cliente.
Caso contrário, o servidor gera automaticamente uma listagem dos arquivos disponíveis no diretório.

---

### 2. Execute o cliente HTTP

```bash
./client/main http://<host>[:porta]/<arquivo>
```

O cliente se conecta ao servidor na porta especificada e envia uma requisição HTTP GET simples, exibindo a resposta recebida.

---

## Funcionamento Interno

- O **servidor** cria um socket, aguarda conexões e responde com o conteúdo solicitado.
- O **cliente** cria um socket, se conecta ao servidor e envia uma requisição no formato HTTP/1.1.
- Caso o arquivo solicitado não exista, o servidor retorna uma mensagem de erro `404 Not Found`.

---

## Requisitos

- GCC ou Clang
- Sistema compatível com POSIX (Linux ou macOS)

---

## Autor: Lucas Henrique Valentim Rocha

Projeto desenvolvido como exemplo didático de comunicação **Cliente-Servidor** via **sockets TCP** e protocolo **HTTP**.

---
