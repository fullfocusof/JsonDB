#include "JsonDB.h"

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_ENTER 13
#define KEY_ESC 27
#define KEY_BACKSPACE 8

HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

void GoToXY(short x, short y)
{
	SetConsoleCursorPosition(hStdOut, { x, y });
}

void ConsoleCursorVisible(bool show, short size)
{
	CONSOLE_CURSOR_INFO structCursorInfo;
	GetConsoleCursorInfo(hStdOut, &structCursorInfo);
	structCursorInfo.bVisible = show;
	structCursorInfo.dwSize = size;
	SetConsoleCursorInfo(hStdOut, &structCursorInfo);
}

int main()
{
	setlocale(LC_ALL, "ru");
	SetConsoleTitle("JSON хранилище");
	ConsoleCursorVisible(false, 100);

	delete_log_file("log.txt");
	log("Программа запущена");

	string db_name;
	cout << "Введите имя базы данных: ";
	cin >> db_name;
	json_db_file_system my_db(db_name);

	Sleep(3000);
	system("cls");

	int active_menu = 0;
	int keyInput;
	bool exitProg = false;

	while (!exitProg)
	{
		int x = 40, y = 12;
		GoToXY(x, y);

		vector<string> menu =
		{
			"Загрузить пример базы данных",
			"Сгенерировать базу данных",
			"Вывод узла базы данных",
			"Обновление узла базы данных",
			"Выход"
		};

		for (int i = 0; i < menu.size(); i++)
		{
			if (i == active_menu)
			{
				SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			}
			else
			{
				SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN);
			}

			GoToXY(x, y++);
			cout << menu[i] << endl;
		}

		keyInput = _getch();
		switch (keyInput)
		{
			case KEY_ESC:
				exit(0);

			case KEY_UP:
			{
				if (active_menu > 0)
				{
					active_menu--;
				}
			}
			break;

			case KEY_DOWN:
			{
				if (active_menu < menu.size() - 1)
				{
					active_menu++;
				}
			}
			break;

			case KEY_ENTER:
			{
				switch (active_menu)
				{
					case 0:
					{
						system("cls");

						try
						{
							my_db.load_example_db();
							cout << "Пример базы данных успешно загружен";
							log("Пример базы данных успешно загружен");
						}
						catch (const exception& e)
						{
							string error = e.what();
							log("Ошибка: " + error);
						}

						printQuit();
					}
					break;

					case 1:
					{
						system("cls");

						try
						{
							my_db.generate_random_json(5);
							cout << "База данных успешно сгенерирована";
							log("База данных успешно сгенерирована");
						}
						catch (const exception& e)
						{
							string error = e.what();
							log("Ошибка: " + error);
						}
						
						printQuit();
					}
					break;

					case 2:
					{
						system("cls");

						log("Запрос данных");
						if (my_db.check_db())
						{
							string node_path;
							cout << "Введите полный путь до узла: ";
							cin >> node_path;
							log("Введен полный путь до узла: " + node_path);

							try
							{
								json node = my_db.get_node(node_path);
								cout << "Получен узел " + node_path + ": " + node.dump(4);
								log("Получен узел " + node_path + ": " + node.dump(4));
							}
							catch (const exception& e)
							{
								string error = e.what();
								cout << "Ошибка: " + error;
								log("Ошибка: " + error);
							}
						}
						else
						{
							cerr << "Данные отсутствуют";
							log("Ошибка запроса данных");
						}

						printQuit();
					}
					break;

					case 3:
					{
						system("cls");

						log("Запрос данных");
						if (my_db.check_db())
						{
							string node_path, key, value;
							json node;
							cout << "Введите полный путь до узла: ";
							cin >> node_path;
							log("Введен полный путь до узла: " + node_path);
							
							cout << "Введите пары <ключ значение> (введите END для завершения):" << endl;
							while (true)
							{
								string key, value;
								cout << "Ключ: ";
								cin >> key;
								if (key == "END") break;

								cout << "Значение: ";
								cin >> value;
								if (value == "END") break;
								
								node[key] = value;
							}
							log("Введены данные: " + node.dump(4));

							try
							{
								my_db.patch_node(node_path, node);
							}
							catch (const exception& e)
							{
								string error = e.what();
								log("Ошибка: " + error);
							}
						}
						else
						{
							cerr << "Данные отсутствуют";
							log("Ошибка запроса данных");
						}

						printQuit();
					}
					break;

					case 4:
					{
						exitProg = true;
					}
					break;
				}
			}
			break;
		}
	}

	log("Программа завершена");
}