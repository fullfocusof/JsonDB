#include "JsonDB.h"

json json_db_file_system::get_node(const string& path)
{
	ifstream file(path + "/diff.json");
	if (!file.is_open())
	{
		log("Ошибка открытия файла: " + path + "/diff.json");
		string error = "Ошибка открытия файла: " + path + "/diff.json";
		throw runtime_error(error);
	}

	json diff,cur_json;
	file >> diff;
	cur_json = diff[0]["value"];
	for (auto& el : cur_json.items())
	{
		if (el.value().is_array()) el.value() = get_node(path + "/" + el.key());
		if (el.value().is_object()) el.value() = get_node(path + "/" + el.key());
	}
	db_data = cur_json;

	return cur_json;
}

json json_db_file_system::patch_node(const string& path, const json& node)
{
	json patch;

	try
	{
		db_data = get_node(path);
		cout << "Полученный узел " + path + ": " << db_data.dump(4) << endl;

		// Создание снимка
		patch = json::diff(db_data, node);

		cout << endl << "Снимок: " << endl;
		cout << patch << endl;

		// Применение снимка
		db_data = db_data.patch(patch);
	}
	catch (const exception& e)
	{
		cerr << "Ошибка: " << e.what() << endl;
	}

	// Проверка обновленного или созданного узла (временно)
	cout << endl << db_data.dump(4);

	// Сохраняем обратно в файл
	save_patch_to_file(path);

	return db_data;
}

void json_db_file_system::load_example_db()
{
	string example_data_01 = R"(
								{
									"NameTable": "table1",
									"DateLimit": "2022-12-01 00:00",
									"data_table": 
									[
										{
											"id": 34,
											"product": "Produkt1"
										},
										{
											"id": 35,
											"product": "Produkt2"
										},
										"string data example inner array",
										345.2345,
										[
											"array",
											"in",
											"array"
										]
									],
									"config_example": 
									{
										"a1": 35,
										"b": 1456.234,
										"c": "random string"
									}
								})";

	ofstream out_file(db_name + ".json");
	if (out_file.is_open())
	{
		out_file << example_data_01;
		out_file.close();
	}
	else
	{
		log("Не удалось открыть файл для записи " + db_name + ".json");
		throw exception("Не удалось открыть файл для записи");
	}

	load_json();
}

json json_db_file_system::create_diff(const json& source, const json& target)
{
	json raw_diff = json::diff(source, target);
	json& value_node = raw_diff[0]["value"];

	for (auto& el : value_node.items())
	{
		if (el.value().is_array()) el.value() = json::array();
		if (el.value().is_object()) el.value() = json::object();
	}

	return raw_diff;
}

void json_db_file_system::save_diff_to_file(const json& diff, const string& path)
{
	ofstream file(path + "/diff.json");
	if (file.is_open())
	{
		log("Открыт файл " + path + "/diff.json");
		file << diff.dump(4);
		log("Данные записаны в файл " + path + "/diff.json");
		file.close();
	}
	else
	{
		cerr << "Не удалось открыть файл для записи: " << path + "/diff.json" << endl;
		log("Ошибка открытия файла: " + path + "/diff.json");
	}
}

void json_db_file_system::recursive_create(const json& j, const string& path)
{
	for (auto& el : j.items())
	{
		const string& key = el.key();
		const json& value = el.value();
		string new_path = path.empty() ? key : path + "/" + key;

		if (value.is_object() || value.is_array())
		{
			// Создание дочерних каталогов и diff файлов
			create_folder(new_path);

			json cur_diff, empty_json;
			cur_diff = create_diff(empty_json, value);
			save_diff_to_file(cur_diff, new_path);

			recursive_create(value, new_path);
		}
	}
}

void json_db_file_system::create_folder(const string& path)
{
	if (!filesystem::exists(path))
	{
		if (filesystem::create_directory(path))
		{
			//cout << "Папка '" + path + "' успешно создана.";
			log("Папка '" + path + "' успешно создана.");
		}
		else
		{
			log("Не удалось создать папку '" + path + "'.");
			cerr << "Не удалось создать папку '" << path << "'." << endl;
		}
	}
	else log("Папка '" + path + "' уже существует.");
}

void json_db_file_system::clear_before_gen()
{
	filesystem::path dirPath = db_name;
	if (filesystem::exists(dirPath) && filesystem::is_directory(dirPath) && !filesystem::is_empty(dirPath))
	{
		for (const auto& entry : filesystem::directory_iterator(dirPath))
		{
			filesystem::remove_all(entry.path());
		}
		log("Директория успешно очищена перед генерацией JSON объекта");
	}
	else log("Директория " + db_name + " не существует либо пуста.");
}

void json_db_file_system::generate_random_json(int depth)
{
	clear_before_gen();

	json j, inner_array;

	inner_array.push_back("array");
	inner_array.push_back("in");
	inner_array.push_back("array");

	// Добавляем примитивные типы
	j["NameTable"] = "randTable";
	j["DateLimit"] = "2022-12-01 00:00";
	j["data_table"] = json::array();
	for (int i = 0; i < depth; ++i)
	{
		j["data_table"].push_back({ {"id", i + 1}, {"product", "Produkt" + to_string(i + 1)} });
	}
	j["data_table"].push_back("string example");
	j["data_table"].push_back(345.2345);
	j["data_table"].push_back(inner_array);

	// Добавляем дочерние объекты
	json inner_data = rand_rec_create_data(depth);
	j["data_table"].push_back(inner_data);
	
	json cfg = rand_rec_create_data(depth);
	j["config_example"] = cfg;

	ofstream out_file(db_name + ".json");
	if (out_file.is_open())
	{
		out_file << j;
		out_file.close();
	}
	else cerr << "Не удалось открыть файл для записи" << endl;

	load_json();
}

bool json_db_file_system::check_db()
{
	return isLoaded;
}

void json_db_file_system::load_json()
{
	// Получение данных из файла
	ifstream db_file(db_name + ".json");
	if (db_file.is_open())
	{
		db_data = json::parse(db_file); // db_file >> db_data;
		db_file.close();
	}

	// Создание и сохранение diff файла
	json empty_json;
	json main_diff = create_diff(empty_json, db_data);
	save_diff_to_file(main_diff, db_name);

	// Рекурсивный обход
	recursive_create(db_data, db_name);

	isLoaded = true;
}

json json_db_file_system::rand_rec_create_data(int depth, int currentDepth)
{
	if (currentDepth >= depth) return json();

	json data;
	data["inner_data_" + to_string(currentDepth)] = rand_rec_create_data(depth, currentDepth + 1);
	data["value"] = "data_at_level_" + to_string(currentDepth);

	return data;
}

void log(const string& message)
{
	auto now = chrono::system_clock::now();
	auto duration = now.time_since_epoch();

	auto seconds = chrono::duration_cast<chrono::seconds>(duration);
	auto microseconds = chrono::duration_cast<chrono::microseconds>(duration) % 1000000;

	time_t time = seconds.count();
	tm* tm = localtime(&time);

	ofstream logFile("log.txt", ios::app); // Открываем файл в режиме добавления
	if (logFile.is_open()) 
	{
		logFile << put_time(tm, "[%Y-%m-%d %H:%M:%S") << "." << setw(6) << setfill('0') << microseconds.count() << "] " << message << endl;
		logFile.close();
	}
	else cerr << "Не удалось открыть файл логирования" << endl;
}

void delete_log_file(const string& filename)
{
	if (remove(filename.c_str()) != 0) cerr << "Не удалось удалить файл: " << filename << endl;
}

void json_db_file_system::save_patch_to_file(const string& path)
{
	ifstream in_file(path + "/diff.json");
	if (!in_file.is_open())
	{
		cerr << "Не удалось открыть файл: " << path + "/diff.json" << endl;
		log("Не удалось открыть файл: " + path + "/diff.json");
	}

	json old_json, patch;
	in_file >> old_json;
	log("Данные загружены в память из файла " + path + "/diff.json");

	patch = json::diff(old_json, db_data);

	ofstream o_file(path + "/diff.json");
	if (!o_file.is_open())
	{
		cerr << "Не удалось открыть файл: " << path + "/diff.json" << endl;
		log("Не удалось открыть файл: " + path + "/diff.json");
	}
	o_file << patch;
	log("Данные выгружены в файл из памяти " + path + "/diff.json");
}

void printQuit()
{
	cout << endl << endl << "Backspace - возврат в меню";

	int answ = _getch();
	bool press = false;

	while (!press)
	{
		if (answ == 8)
		{
			press = true;
			log("Возврат в меню");
			break;
		}
		else answ = _getch();
	}

	system("cls");
}