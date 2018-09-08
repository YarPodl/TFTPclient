/**
	\file
	\brief Файл реализации класса
*/

#include "client.h"


#include <string.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")


#define MaxLengthPacket 516
#define PORT 69
#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERRO 5





void client::Get()
{

	openSocket();		// открываем сокет

	// длина первого запроса
	unsigned long size = 4 + strlen(fileName) + strlen(Mode);	


	char* value = getFirstReq(RRQ, size);	// формирование первого сообщения

	sockaddr_in *addressServer = getsockaddr_in();	// заполнение адреса сервера

	// отправка первого сообщения
	int sent_bytes = sendto(my_sock, (const char*)value, size,
		0, (sockaddr*)addressServer, sizeof(sockaddr_in));

	chekSended(sent_bytes, size);

	std::ofstream file;	//файл
	file.open(fileName, std::ios::binary);	// открытие файла

	getting(file, addressServer);	// прием

	file.close();			// закрытие файла
}

void client::Put()
{
	openSocket();	// открываем сокет

	unsigned long size = 4 + strlen(fileName) + strlen(Mode);	// длина первого запроса

	char* value = getFirstReq(WRQ, size);	// формирование первого сообщения

	sockaddr_in *addressServer = getsockaddr_in();	// заполнение адреса сервера

	// отправка первого сообщения
	int sent_bytes = sendto(my_sock, (const char*)value, size,
		0, (sockaddr*)addressServer, sizeof(sockaddr_in));

	chekSended(sent_bytes, size);	// Проверка, отправлено ли


	std::ifstream file;	// файл
	file.open(fileName, std::ios::binary);	// открытие файла
	if (!file.is_open())
	{
		throw new std::exception("Файл не удалось открыть");
	}

	putting(file, addressServer);	// отправка

	file.close();			// закрытие файла
}

client::client(const char *fName, const char * servaddr)
{
	serveraddr = servaddr;
	fileName = fName; WSADATA WsaData;

	if (WSAStartup(MAKEWORD(2, 2), &WsaData) != NO_ERROR)
	{
		throw new std::exception("Работа с сокетами не запущена");
	}
}

client::~client()
{
	if (my_sock)
	{
		closesocket(my_sock);	// закрытие сокета
	}
	WSACleanup();			// закрытие работы с сокетами
}

void client::chekSended(int sent_bytes, int size)
{
	// Проверка, отправлено ли
	if (sent_bytes != size)	// Проверка, отправлено ли
	{
		throw new std::exception("Сообщение не удалось отправить");
	}
}


char* client::getFirstReq(int type, unsigned long size)
{
	char* value = new char[size];	// формирование первого сообщения
	value[0] = (char)0;				//
	value[1] = (char)type;			//
	strcpy(value + 2, fileName);	//
	strcpy(value + 3 + strlen(fileName), Mode);
	return value;
}


sockaddr_in* client::getsockaddr_in()
{
	sockaddr_in *addressServer = new sockaddr_in();				// заполнение адреса сервера
	addressServer->sin_family = AF_INET;		//
	inet_pton(AF_INET, serveraddr, &addressServer->sin_addr.s_addr);//
	addressServer->sin_port = htons(PORT);	//
	return addressServer;
}


void client::putting(std::ifstream &file, sockaddr_in *addressServer)
{
	char* value = getFirstReq(WRQ, 4 + strlen(fileName) + strlen(Mode));	// формирование первого сообщения
	unsigned long size;
	int notReq = 0;		// Количество итераций подряд в которых сервер не ответил
	int received_bytes;		// количество отправленных байт
	unsigned char packet_data[4];	// Массив байт для присылания
	sockaddr_in from;	 // Адрес сервера (который пришлет сам сервер)
	socklen_t fromLength = sizeof(from);
	int numberBlock = 0;	// номер блока
	size = MaxLengthPacket;	// размер отправляемого блока
	while (size == MaxLengthPacket)	// Отправляем, пока не отправим последний блок (< 512 байт)
	{
		Sleep(100);	// ждем 0.1 секунды

					// Попытка принять сообщение
		received_bytes = recvfrom(my_sock, (char*)packet_data, MaxLengthPacket,
			0, (sockaddr*)&from, &fromLength);

		// если не принято
		if (received_bytes <= 0)
		{
			// отправляем еще раз
			int sent_bytes = sendto(my_sock, (const char*)value, size,
				0, (sockaddr*)addressServer, sizeof(sockaddr_in));

			chekSended(sent_bytes, size);
			// если сервер не ответил 3 раза
			if (notReq == 3)
			{
				throw new std::exception("Сообщение не принято");
			}
			notReq++;
			continue;
		}
		if (packet_data[1] == ACK)	// Если это сообщение подтверждения
		{
			if (packet_data[2] * 256 + packet_data[3] != numberBlock)	// если не совпадает номер блока
			{
				continue;
			}
			numberBlock++;
		}
		else
		{
			if (packet_data[1] == ERRO)	// если сообщение ошибки
			{
				throw new std::exception((const char *)(packet_data + 4));// вывод текста ошибки
			}
			throw new std::exception("Сервер не подтвердил");
		}



		// Меняем порт на тот, который прислал сервер
		addressServer->sin_port = from.sin_port;


		// формирование пакета данных
		char* value = new char[MaxLengthPacket];
		value[0] = (char)0;
		value[1] = (char)DATA;	// тип сообщение
		value[2] = (char)(numberBlock / 256);	// номер блока
		value[3] = (char)(numberBlock % 256);	//
		for (size = 4; size < MaxLengthPacket; size++)	// читаем файл
		{
			value[size] = file.get();	// читаем символ
			if (file.eof())		// если файл кончился
			{
				break;
			}
		}

		// отправляем пакет
		int sent_bytes = sendto(my_sock, (const char*)value, size,
			0, (sockaddr*)addressServer, sizeof(sockaddr_in));

		chekSended(sent_bytes, size);
	}
}


void client::getting(std::ofstream &file, sockaddr_in *addressServer)
{
	int notReq = 0;	// Количество итераций подряд в которых сервер не ответил
	unsigned char packet_data[MaxLengthPacket];	// массив для присылаемых данных
	sockaddr_in from;	// Адрес сервера (который пришлет сам сервер)
	socklen_t fromLength = sizeof(from);
	int received_bytes = MaxLengthPacket;
	int numberBlock = 1;	// номер блока
	while (true)	// пока не пришло последнее сообщение
	{
		Sleep(100);	// ждем 0.1 секунды

					// Попытка принять сообщение
		received_bytes = recvfrom(my_sock, (char*)packet_data, MaxLengthPacket,
			0, (sockaddr*)&from, &fromLength);

		// если не принятр
		if (received_bytes <= 0)
		{
			if (notReq == 5)	// если сервер не ответил 5 раз подряд
			{
				throw new std::exception("Сообщение не принято");
			}
			notReq++;
			continue;
		}
		notReq = 0;

		// если присланы данные
		if (packet_data[1] == DATA)
		{
			// если номер пакета не совпадает
			if (packet_data[2] * 256 + packet_data[3] != numberBlock)
			{
				continue;
			}
		}
		else
		{
			if (packet_data[1] == ERRO)	// если сообщение ошибки
			{
				throw new std::exception((const char *)(packet_data + 4));// вывод текста ошибки
			}
			throw new std::exception("Сервер не подтвердил");
		}

		// записываем полученные данные в файл
		file.write((const char*)packet_data + 4, received_bytes - 4);

		// Меняем порт на тот, который прислал сервер
		addressServer->sin_port = from.sin_port;

		// формирование сообщение о подтверждении
		unsigned long size = 4;
		char* value = new char[size];
		value[0] = (char)0;
		value[1] = (char)ACK;		// Тип сообщения
		value[2] = (char)(numberBlock / 256);	// Номер блока
		value[3] = (char)(numberBlock % 256);	// 

												// Отправка подтверждения
		int sent_bytes = sendto(my_sock, (const char*)value, size,
			0, (sockaddr*)addressServer, sizeof(sockaddr_in));

		chekSended(sent_bytes, size);

		numberBlock++;	// Переход к следующему блоку

		if (received_bytes != MaxLengthPacket)
		{
			break;
		}
	}
}



void client::openSocket(int port)
{
	//открытие сокета
	my_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (my_sock == INVALID_SOCKET)
	{
		WSACleanup();
		throw new std::exception("Ошибка сокета");
	}

	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	bindSocket(port);
	DWORD nonBlocking = 1;
	ioctlsocket(my_sock, FIONBIO, &nonBlocking);
}

void client::bindSocket(int port)
{
	address.sin_port = htons((unsigned short)port);

	if (bind(my_sock, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
	{
		throw new std::exception("Ошибка связывания сокета");
	}
}
