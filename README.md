# NET
- **Select:** Server using select. Here we send 10 data.txt with a specific protocol and we read it with a specific parsing (protocol) and then we write it in 10 files (file.txt)
  - **For a simple connection:**
    ```bash
    $ nc 127.0.0.1 9034
    ```
  - **Connect to the server and send multiple txt files:**
    ```bash
    $ cat data.0.txt|nc 127.0.0.1 9034 & cat data.1.txt|nc 127.0.0.1 9034 & cat data.2.txt|nc 127.0.0.1 9034 & cat data.3.txt|nc 127.0.0.1 9034 & cat data.4.txt|nc 127.0.0.1 9034 & cat data.5.txt|nc 127.0.0.1 9034 & cat data.6.txt|nc 127.0.0.1 9034 & cat data.7.txt|nc 127.0.0.1 9034 & cat data.8.txt|nc 127.0.0.1 9034 & cat data.9.txt|nc 127.0.0.1 9034
    ```

- **Select_threads:** Server using select but also with threads to manage the information incoming from de data files to write it into new files (file.txt). On the other hand, mutex has been included to implement locks for every one of the threads.

  - **For a simple connection:**
    ```bash
    $ nc 127.0.0.1 9034
    ```
  - **Connect to the server and send multiple txt files:**
    ```bash
    $ cat data.0.txt|nc 127.0.0.1 9034 & cat data.1.txt|nc 127.0.0.1 9034 & cat data.2.txt|nc 127.0.0.1 9034 & cat data.3.txt|nc 127.0.0.1 9034 & cat data.4.txt|nc 127.0.0.1 9034 & cat data.5.txt|nc 127.0.0.1 9034 & cat data.6.txt|nc 127.0.0.1 9034 & cat data.7.txt|nc 127.0.0.1 9034 & cat data.8.txt|nc 127.0.0.1 9034 & cat data.9.txt|nc 127.0.0.1 9034
    ```
- **CRUD:** Here we can Create, Read, Update and Delete fields of an Distributed DataBase. There is an implementation of a storage node (nodo de almacenamiento NA) and a parent node (nodo principal NP).
  - **Command** to create something, me pass a field and a description (data)
     ```bash
    $ echo c00006alumno00005messi | nc 127.0.0.1 9034

    ```
