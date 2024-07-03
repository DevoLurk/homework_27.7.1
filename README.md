Для работы серверной нужна пустая база данных MySQL с любым названием и odbc driver.

Инструкция первого старта сервера:
1. Создать новую базу данных MySQL с именем "ваше_название_базы".
2. Сервер хранит парамерты подключения к базе данных в файле config/dbconnection.conf
   в фотрмате:
    ```
    DRIVER={MySQL ODBC 8.4 ANSI Driver};
    SERVER=localhost;
    PORT=3306;
    DATABASE=test1;
    UID=your_username;
    PWD=your_pass;
    ```
   При первом запуске "ваше_название_базы" будет записано config/dbconnection.conf автоматически
   Версию драйвера, сервер, пользователя базы данных и его пароль надо указать вручную. 
   
3. Запустить .exe файл, в параметрах запуска указать "--ваше_название_базы".  
   Пример: cmd> c:\CPP projects\projectname.exe --database_name
4. Программа сама создаст все нужные таблицы в базе данных  
   Для последующих запусков пограммы параметры указывать не нужно

---------------------------------------------------------------------------------------------------  
Возможные проблемы:
1. Строки в файле dbconnection.conf очень чувствительны к пробелам  
   После ; не должно быть никаких пробелов  
   В строке DRIVER= после { не должно быть пробелов и перед } тоже
2. [РЕШЕНО]Имя пользователя не должно привышать 30 символов в длинну (я забыл добавить ограничение в программе)  
3. [РЕШЕНО]Имя пользователя не должно сожержать спец символов
4. Сообщения не должны быть длиннее 65535 символов, хотя вряд-ли кто то столько набирет
