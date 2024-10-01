#include<iostream>
#include<sqlite3.h>
#include<Windows.h>
#include<string>
#include<map>
#include<stdexcept>

using namespace std;

enum struct Priority {
	Hight,
	Medium,
	Low
};

class Task {
private:
	int id;
	string title;
	string description;
	Priority priority;
	bool isCompleted;
	string due_date;
public:

	Task() : id(0), title(""), description(""), priority(Priority::Low), isCompleted(false), due_date("") {}
	Task(int id,string title,string description,Priority priority,string due_date):
		id(id),title(title),description(description),priority(priority), isCompleted(false),due_date(due_date)
	{}

	int getId() const {
		return id;
	}
	string getTitle()const {
		return title;
	}
	string getDescription()const {
		return description;
	}
	Priority getPriority()const {
		return priority;
	}
	bool getStatus()const {
		return isCompleted;
	}
	string getDue_date()const {
		return due_date;
	}
	string getPriorityString() const {
		return this->getPriority() == Priority::Hight ? "Hight" : this->getPriority() == Priority::Low ? "Low" : this->getPriority() == Priority::Medium ? "Medium" : "Unknow";
	}

	void setId(int id) {
		this->id = id;
	}
	void setTitle(string title) {
		this->title = title;
	}
	void setDescription(string description) {
		this->description = description;
	}
	void setPriority(Priority priority) {
		this->priority = priority;
	}
	void setStatus(bool status) {
		this->isCompleted = status;
	}
	void setDue_date(string due_date) {
		this->due_date = due_date;
	}
};

class TaskManager {
private:
	map<int, Task> tasks;
	int nextId;
public:
	TaskManager(): nextId(1) {}
	void addTask(const Task& task) {
		Task newTask = task;
		newTask.setId(nextId++);
		tasks[newTask.getId()] = newTask;
	}
	void removeTask(int id) {
		tasks.erase(id);
	}
	map<int,Task> getMap() {
		return tasks;
	}
	Task getTaskById(int id) {
		if (tasks.find(id) == tasks.end())
			throw runtime_error("Error: incorrect id");

		return tasks[id];
	}
	void editTask(int id, const Task& task) {
		if (tasks.find(id) != tasks.end()) {
			tasks[id] = task;
		}
		else {
			std::cout << "Task is not found!" << std::endl;
		}
	}

	void viewTask() {
		for (auto& task : tasks) {
			const Task& task1 = task.second;
			std::cout << "ID: " << task1.getId() <<
				", title: " << task1.getTitle() <<
				", priority: " << ([&]() {
				return task1.getPriority() == Priority::Hight ? "High" :
					task1.getPriority() == Priority::Low ? "Low" :
					task1.getPriority() == Priority::Medium ? "Medium" : "Unknown";
				})() << ", completed: " << (task1.getStatus() ? "Yes" : "No") <<
						", due date: " << task1.getDue_date() << std::endl;
		}
	}
};

class Database {
private:
	sqlite3* db;
	const char* dbName;
public:
	Database(const char* databaseName) : db(nullptr), dbName(databaseName) {
		if (sqlite3_open(dbName, &db) != SQLITE_OK) {
			std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
		}
	}
	~Database() {
		if (db) 
			sqlite3_close(db);
		
	}

	void creatTable() {
		const char* sqlCreateTable = "CREATE TABLE IF NOT EXISTS tasks ("
			"id INTEGER PRIMARY KEY AUTOINCREMENT,"
			"title TEXT NOT NULL,"
			"description TEXT,"
			"priority TEXT,"
			"isCompleted INTEGER,"
			"due_date TEXT);";
		char* errorMessage = nullptr;
		if (sqlite3_exec(db, sqlCreateTable, 0, 0, &errorMessage)!=SQLITE_OK) {
			string err = string(errorMessage);
			sqlite3_free(errorMessage);
			throw runtime_error("Failed to create table: " + err);
		}
		std::cout << "Table created successfully!" << std::endl;
	}

	void insertTask(const Task& task) {
		const char* sqlInsert = "INSERT INTO tasks (title, description, priority, isCompleted, due_date) VALUES (?, ?, ?, ?, ?);";
		sqlite3_stmt* stmt;
		
		if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, 0) != SQLITE_OK) {
			throw runtime_error("Failed to prepare statement : " + sqlite3_errcode(db));
		}

		sqlite3_bind_text(stmt, 1, task.getTitle().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, task.getDescription().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, task.getPriorityString().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 4, task.getStatus() ? 1 : 0);
		sqlite3_bind_text(stmt, 5, task.getDue_date().c_str(), -1, SQLITE_STATIC);

		if (sqlite3_step(stmt) != SQLITE_DONE) {
			sqlite3_finalize(stmt);
			throw runtime_error( "Failed to insert task: " + string(sqlite3_errmsg(db)));
		}
		else {
			std::cout << "Task inserted successfully!" << std::endl;
		}
		sqlite3_finalize(stmt);
	}
	void deleteTask(int id) {
		const char* sqlDelete = "DELETE FROM tasks WHERE id = ?;";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db, sqlDelete, -1, &stmt, 0) != SQLITE_OK) {
			throw runtime_error("Failed to prepare delete statement: " + string(sqlite3_errmsg(db)));
		}
		sqlite3_bind_int(stmt, 1, id);
		if (sqlite3_step(stmt)!=SQLITE_DONE) {
			throw runtime_error("Failed to delete task: " + string(sqlite3_errmsg(db)));
		}
		std::cout << "Task deleted successfully!" << std::endl;
		sqlite3_finalize(stmt);
	}
	void updateTask(const Task& task) {
		const char* sqlUpdate = "UPDATE tasks SET title = ?, description = ?, priority = ?, isCompleted = ?, due_date =? WHERE id = ?;";
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, 0) != SQLITE_OK) {
			throw runtime_error("Failed to prepare update statement: " + string(sqlite3_errmsg(db)));
		}
		sqlite3_bind_text(stmt, 1, task.getTitle().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, task.getDescription().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, task.getPriorityString().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 4, task.getStatus() ? 1 : 0);
		sqlite3_bind_text(stmt, 5, task.getDue_date().c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 6, task.getId());

		if (sqlite3_step(stmt) != SQLITE_DONE) {
			sqlite3_finalize(stmt);
			throw runtime_error("Failed to update task: " + string(sqlite3_errmsg(db)));
		}
		std::cout << "Task updated successfully" << std::endl;
		sqlite3_finalize(stmt);
	}

	void fetchTasks() {
		const char* sqlSelect = "SELECT id, title, description, priority, isCompleted, due_date FROM tasks;";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, 0) != SQLITE_OK) {
			throw runtime_error( "Failed to prepare fetch statement: " + string(sqlite3_errmsg(db)));
		}
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			int id = sqlite3_column_int(stmt, 0);
			std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
			std::string description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
			std::string priority = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
			bool isCompleted = sqlite3_column_int(stmt, 4);
			std::string due_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

			std::cout << "ID: " << id
				<< ", Title: " << title
				<< ", Description: " << description
				<< ", Priority: " << priority
				<< ", Completed: " << (isCompleted ? "Yes" : "No")
				<< ", Due Date: " << due_date
				<< std::endl;
		}

		sqlite3_finalize(stmt);
	}
};


int main() {
	TaskManager tm;  // Менеджер задач
	Database db("tasks.db");  // База данных для хранения задач
	try {
		db.creatTable();  // Создание таблицы, если она не существует

		char choice = '0';
		while (choice != '6') {
			std::cout << "Menu: \n";
			std::cout << "1. Create task\n";
			std::cout << "2. View all tasks\n";
			std::cout << "3. Update task\n";
			std::cout << "4. Delete task\n";
			std::cout << "5. View tasks from Database\n";
			std::cout << "6. Exit\n";
			std::cout << "Enter your choice: ";
			std::cin >> choice;

			switch (choice) {
			case '1': {
				int id;
				std::string title, description, due_date;
				int priority;
				bool completed = false;

				std::cout << "Enter task ID: ";
				std::cin >> id;
				std::cin.ignore();  // Игнорируем остаток строки после ID

				std::cout << "Enter title: ";
				std::getline(std::cin, title);

				std::cout << "Enter description: ";
				std::getline(std::cin, description);

				std::cout << "Enter due date (YYYY-MM-DD): ";
				std::getline(std::cin, due_date);

				std::cout << "Enter priority (0 = Low, 1 = Medium, 2 = High): ";
				std::cin >> priority;

				// Создание задачи
				Task newTask(id, title, description, static_cast<Priority>(priority), due_date);
				tm.addTask(newTask);  // Добавление задачи в TaskManager
				db.insertTask(newTask);  // Добавление задачи в базу данных
				std::cout << "Task added successfully!\n";
				break;
			}
			case '2': {
				std::cout << "Tasks in TaskManager:\n";
				tm.viewTask();
				break;
			}
			case '3': {
				int id;
				std::cout << "Enter task ID to update: ";
				std::cin >> id;
				std::cin.ignore();

				Task task = tm.getTaskById(id);
				std::string new_title, new_description, new_due_date;
				int new_priority;

				std::cout << "Enter new title: ";
				std::getline(std::cin, new_title);

				std::cout << "Enter new description: ";
				std::getline(std::cin, new_description);

				std::cout << "Enter new due date (YYYY-MM-DD): ";
				std::getline(std::cin, new_due_date);

				std::cout << "Enter new priority (0 = Low, 1 = Medium, 2 = High): ";
				std::cin >> new_priority;

				// Обновляем задачу
				task.setTitle(new_title);
				task.setDescription(new_description);
				task.setDue_date(new_due_date);
				task.setPriority(static_cast<Priority>(new_priority));

				tm.editTask(id, task);  // Обновление задачи в TaskManager
				db.updateTask(task);    // Обновление задачи в базе данных
				std::cout << "Task updated successfully!\n";
				break;
			}
			case '4': {
				int id;
				std::cout << "Enter task ID to delete: ";
				std::cin >> id;

				tm.removeTask(id);  // Удаление задачи из TaskManager
				db.deleteTask(id);  // Удаление задачи из базы данных
				std::cout << "Task deleted successfully!\n";
				break;
			}
			case '5': {
				std::cout << "Tasks from Database:\n";
				db.fetchTasks();  // Вывод задач из базы данных
				break;
			}
			case '6':
				std::cout << "Exiting...\n";
				break;
			default:
				std::cout << "Invalid choice! Please try again.\n";
				break;
			}
		}
	}
	catch (runtime_error& e) {
		cout << "Error: " << e.what() << std::endl;
	}
	return 0;
}