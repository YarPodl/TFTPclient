/**
	\file
	\brief Заголовочный файл с описанием класса
*/

#pragma once


#include <winsock2.h> // Включаем использование сокетов
#include <exception>	
#include <fstream>

/// Класс, реализующий функции TFTP клиента
class client
{
private:
	/// Переменная для хранения сокета
	SOCKET my_sock;
	/// Модификатор, определяющий способ пересылки файлов
	const char *Mode = "octet";
	/// Адрес, к которому привязывается сокет
	sockaddr_in address;
	/**
	\brief Открытие сокета
	\param port Порт, к которому подключается создаваемый сокет
	\throw exception Если создание не удалось
	*/
	void openSocket(int port = 0);
	/**
	\brief Связывание сокета с адресом
	\param port Порт, с которомым связывается сокет
	\throw exception Если связывание не удалось
	*/
	void bindSocket(int port = 0);
	/**
	\brief Проверка отправлено ли сообщение
	\param sent_bytes Количество отправленных байт
	\param size Количество байт, которые должны были быть отправлены
	\throw exception Если количество отправленных байт не совпадает с правильным
	*/
	void chekSended(int sent_bytes, int size);
	/**
	\brief Получение файла с сервера
	\param file Файловый поток, в который сохраняется файл
	\param addressServer Адрес сервера, с которого принимается файл
	\throw exception Если принять не удалось
	*/
	void getting(std::ofstream &file, sockaddr_in *addressServer);
	/**
	\brief Отправка файла на сервер
	\param file Файловый поток, из которого читается файл
	\param addressServer Адрес сервера, на который отправляется файл
	\throw exception Если отправить не удалось
	*/
	void putting(std::ifstream &file, sockaddr_in *addressServer);
	/**
	\brief Формирование первого запроса к серверу
	\param type Тип создаваемого запроса
	\param size Длина создаваемого запроса
	\return Сообщение для сервера
	*/
	char* getFirstReq(int type, unsigned long size);
	/**
	\brief Формирование и заполнение адреса 
	\return Структура содержащая адрес
	*/
	sockaddr_in* getsockaddr_in();
	/// имя файла
	const char *fileName;			
	/// адрес сервера
	const char * serveraddr;	
public:
	/**
	\brief Получение и сохранение файла с сервера
	*/
	void Get();			
	/**
	\brief Чтение и отправка файла на сервер
	*/
	void Put();			
	// Конструктор
	/**
	\brief Конструктор
	\param fName Название файла
	\param servaddr Адрес сервера, например 255.255.255.255
	*/
	client(const char *fName, const char * servaddr);			
	/// Деструктор
	~client();
};

