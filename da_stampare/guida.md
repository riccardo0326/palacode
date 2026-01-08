# 📄 PIPE + COMANDI UNIX (SISTEMI)

---

## 🧱 Struttura generale

* **1 comando = 1 processo**
* **1 `|` = 1 `pipe()`**
* Comunicazione solo tramite **stdin / stdout**

```c
int fd[2];
pipe(fd);
// fd[0] → lettura
// fd[1] → scrittura
```

---

## 🔍 `grep` — filtro

Cerca righe che contengono un pattern.

### Shell

```bash
grep PATTERN file
```

### In pipe

* legge da file o stdin
* scrive su stdout

### In C

```c
close(1);
dup(fd[1]);
close(fd[1]);
close(fd[0]);

execlp("grep", "grep", pattern, file, NULL);
```

---

## 🔃 `sort` — ordinamento

Ordina righe di testo.

### Shell

```bash
sort
sort -n
sort -r
sort -u
```

### In pipe

* legge da stdin
* scrive su stdout

### In C

```c
close(0);
dup(fd_in[0]);
close(fd_in[0]);

close(1);
dup(fd_out[1]);
close(fd_out[1]);

execlp("sort", "sort", "-n", NULL);
```

---

## 📤 `head` — prime righe

Mostra le prime N righe.

### Shell

```bash
head -n N
```

### In pipe

* legge da stdin
* scrive su stdout

### In C

```c
close(0);
dup(fd[0]);
close(fd[0]);
close(fd[1]);

execlp("head", "head", "-n", N, NULL);
```

---

## 📥 `tail` — ultime righe

Mostra le ultime N righe.

### Shell

```bash
tail -n N
```

### In pipe

* legge da stdin
* scrive su stdout

### In C

```c
close(0);
dup(fd[0]);
close(fd[0]);
close(fd[1]);

execlp("tail", "tail", "-n", N, NULL);
```

---

## ✂️ `cut` — estrazione colonne

Estrae campi da righe strutturate.

### Shell

```bash
cut -d":" -f1,3
```

### In pipe

* legge da stdin
* scrive su stdout

### In C

```c
close(0);
dup(fd_in[0]);
close(fd_in[0]);

close(1);
dup(fd_out[1]);
close(fd_out[1]);

execlp("cut", "cut", "-d:", "-f1,3", NULL);
```

---

## 👨 Processo padre

Chiude tutte le pipe e attende i figli.

```c
close(fd[0]);
close(fd[1]);

waitpid(pid1, NULL, 0);
waitpid(pid2, NULL, 0);
```

---

## 🧪 Output su socket o file

Per redirigere l’output finale:

```c
close(1);
dup(ns);   // stdout → socket
close(ns);
```

---