#include <sys/stat.h>   //для mkfifo
#include <stdio.h>
#include <string.h>

#include <unistd.h>     //Для open
#include <fcntl.h>      //Для O_WRONLY, O_RDONLY


#define FIRST  "/tmp/first_pipe"
#define SECOND "/tmp/second_pipe"
#define BUFSIZE 50

void process_conversation(int file_descrp2, int file_descrp1) {             //Диалог процесса
        ssize_t bytes_read = 0;     //Сколько символов в сообщении
        char buf[BUFSIZE];  //Куда будем считывать сообщение процессов
        while (1){
            memset(buf, '\0', BUFSIZE);                                     //Заполняем все байты терминирующим нулем
            bytes_read = read(file_descrp2, buf, BUFSIZE - 1);              //Считываем BUFSIZE - 1 символов из 2 файла fifo
            if (bytes_read <= 0) { 
                perror("read");                                             //Если не получилось считать
                close(file_descrp1);                                        //Закрываем файловый дескриптор
                close(file_descrp2);                                        //Закрываем файловый дескриптор
                remove(SECOND);                                             //Удаляем 2 файл fifo
                remove(FIRST);                                              //Удаляем 1 файл fifo
                return;
            }
            printf("Incomming message (%ld): %s\n", bytes_read, buf);
            char message [] = "test message";                                                               //Тестовая фраза
            ssize_t bytes_written = write(file_descrp1, message, strlen(message)+1);                        //Отправляем её в 1 fifo
            if (bytes_written == -1){                                                                       //Проверка на ошибку записи
                perror("write");
                close(file_descrp1);                                         //Закрываем файловый дескриптор
                close(file_descrp2);                                         //Закрываем файловый дескриптор
                return;
            }
        }
}

int main(){
    int file_descrp1 = 0;   //Файловый дескриптор для дальнейшей работы с 1 файлом
    int file_descrp2 = 0;
    if (mkfifo(FIRST, 0666) == -1) {     //Создаем fifo файл для первого процесса (он в него записывает, читает из второго fifo файла)
        perror("mkfifo1");                            //Проверка, если не получилось создать
        return 1;
    }
    if (mkfifo(SECOND, 0666) == -1){     //Создаем fifo файл для второго процесса (можно проверку объединить в одну, но так по мне читабельнее)
        perror("mkfifo2");                            //Проверка, если не получилось создать
        return 1;
    }

    printf("first_pipe and second_pipe was created succesfully\n");
    if (fork() == 0){       //Блок кода для дочернего процесса (мы его считаем первым)
        if ( (file_descrp1 = open(FIRST, O_WRONLY)) == -1){      //Открываем первый fifo для записи
            perror("open1");
            return 2;
        }
        if ( (file_descrp2 = open(SECOND, O_RDONLY)) == -1){     //Открываем второй fifo для чтения
            perror("open2");
            return 2;
        }
        process_conversation(file_descrp1, file_descrp2);       //Общение для первого процесса
    }else{              //Блок кода для родительского процесса (его считаем вторым)
        if ( (file_descrp2 = open(SECOND, O_WRONLY)) == -1){      //Открываем второй fifo для записи
            perror("open2");
            return 2;
        }
        if ( (file_descrp1 = open(FIRST, O_RDONLY)) == -1){     //Открываем первый fifo для чтения
            perror("open1");
            return 2;
        }
        process_conversation(file_descrp2, file_descrp1);       //Общение для второго процесса
    }

    close(file_descrp1);                                        //Закрываем файловый дескриптор
    close(file_descrp2);                                        //Закрываем файловый дескриптор
    remove(SECOND);                                             //Удаляем 2 файл fifo
    remove(FIRST);                                              //Удаляем 1 файл fifo

    return 0;
}
