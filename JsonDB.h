#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>

#include <vector>
#include <string>

#include <cctype>
#include <random>

#include <Windows.h>
#include <conio.h>
#include <chrono>

#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::ordered_json;

class abstract_json_db
{
public:
	abstract_json_db(const string& _db_name) :db_name(_db_name) {};
	virtual json get_node(const string& path) = 0;
	virtual json patch_node(const string& path, const json& node) = 0;
protected:
	const string db_name;
};

class json_db_file_system : public abstract_json_db
{
public:
	json_db_file_system(const string& _db_name) : abstract_json_db(_db_name)
	{
		// Создание корневого каталога
		create_folder(db_name);
		isLoaded = false;
	}
	
	json get_node(const string& path) override;
	json patch_node(const string& path, const json& node) override;

	void load_example_db();
	
	void generate_random_json(int depth);
	bool check_db();
	void load_json();

protected:
	json create_diff(const json& source, const json& target); // Создание diff файла
	void save_diff_to_file(const json& diff, const string& path); // Сохранение diff файла
	void save_patch_to_file(const string& path);

	void create_folder(const string& path);
	void recursive_create(const json& j, const string& path);
	json rand_rec_create_data(int depth, int currentDepth = 0);

	void clear_before_gen();

private:
	json db_data;
	bool isLoaded;

};

void log(const string& message);
void delete_log_file(const string& filename);
void printQuit();