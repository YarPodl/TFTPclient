// TFTPclient.cpp: определяет точку входа для консольного приложения.
//

/**
	\mainpage Клиент TFTP
	\authors Живодрова Анастасия
	\file
	\brief Главный файл проекта, содержащий интерфейс пользователя
*/



#include "client.h"
#include <iostream>
#include <string>
using namespace std;

int main(int argc, char* argv[])
{
	SetConsoleCP(1251);		// установка кодировки для ввода
	SetConsoleOutputCP(1251);// установка кодировки для вывода
	string serveraddr = "127.0.0.1";
	string fileName = "1.txt";
	
	cout << "Текущий адрес сервера: " << serveraddr << endl;
	cout << "Текущий файл: " << fileName << endl;
	cout << "Введите:"
		<< endl << "1 - чтобы изменить адрес сервера"
		<< endl << "2 - чтобы изменить файл"
		<< endl << "3 - чтобы отправить файл на сервер"
		<< endl << "4 - чтобы получить файл с сервера"
		<< endl << "5 - чтобы завершить работу программы"
		<< endl;

	char n;
	bool b = true;
	while (b)
	{
		try
		{
			cin >> n;
			client c(fileName.c_str(), serveraddr.c_str());
			switch (n)
			{
			case '1':
				cout << "Введите новый адрес:" << endl;
				getline(cin, serveraddr);		// Сброс остатка строки
				getline(cin, serveraddr);		// Ввод строки
				cout << "Текущий адрес сервера: " << serveraddr << endl;
				break;
			case '2':
				cout << "Введите новый файл:" << endl;
				getline(cin, fileName);		// Сброс остатка строки
				getline(cin, fileName);		// Ввод строки
				cout << "Текущий файл: " << fileName << endl;
				break;
			case '3':
				c.Put();
				cout << "Файл отправлен успешно" << endl;
				break;
			case '4':
				c.Get();
				cout << "Файл получен успешно" << endl;
				break;
			case '5':
				b = false;
				break;
			default:

				break;
			}
		}
		catch (exception *ex)
		{
			cout << ex->what() << endl;
		}
		catch (...)
		{
			cout << "Произошла ошибка!" << endl;
		}
	}
	
	system("pause");
	return 0;
}

