#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "lib.h"
#include "admin_panel.h"

using namespace std;

AdminPanel adminPanel = AdminPanel(-1);

void menu() {
    while (true) {
        if (adminPanel.IsChosedTerminal()) {
            Terminal terminal = adminPanel.GetChosedTerminal();
            cout << "[-] Меню терминала с PID " << terminal.pid << endl;
            cout << "[11] Текущее состояние" << endl;
            cout << "[12] Изменить состояние" << endl;

            cout << "[21] Текущее ПО" << endl;
            cout << "[22] Установить ПО" << endl;
            cout << "[23] Обновить ПО" << endl;
            cout << "[24] Удалить ПО" << endl;

            cout << "[31] Текущие пользователи" << endl;
            cout << "[32] Добавить пользователя" << endl;
            cout << "[33] Изменить права пользователя" << endl;
            cout << "[34] Удалить пользователя" << endl;

            cout << "[41] Пользователь текущего сеанса" << endl;

            cout << "[0] Выйти" << endl;
            int action;
            cin >> action;
            switch (action) {
                case 11: {
                    TerminalState state = adminPanel.GetTerminalState(terminal.pid);
                    cout << "Состояние текущего терминала (PID " << terminal.pid << "): "
                         << TerminalStateToString(state) << endl;
                    break;
                }
                case 12: {
                    cout << "Выберите новое состояние (или -1 чтобы не выбирать):" << endl;
                    cout << "[1] Online" << endl;
                    cout << "[2] Offline" << endl;

                    int chose;
                    cin >> chose;

                    TerminalState state = TerminalState::Undefined;
                    switch (chose) {
                        case 1:
                            state = TerminalState::Online;
                            break;
                        case 2:
                            state = TerminalState::Offline;
                            break;
                        case -1:
                            continue;
                            break;
                        default:
                            cout << "Некорректное значение" << endl;
                            break;
                    }

                    if (!adminPanel.SetTerminalState(adminPanel.GetChosedTerminal().pid, state)) {
                        cout << "Изменить состояние не удалось" << endl;
                    }
                    break;
                }
                case 21: {
                    cout << "Текущее установленное ПО:" << endl;
                    for (auto &software: adminPanel.GetSoftware(terminal.pid)) {
                        cout << software.name << " (v " + software.version + ")" << endl;
                    }
                    break;
                }
                case 22: {
                    cout << "Установка нового ПО" << endl;

                    Software software;
                    cout << "Заполните данные" << endl;

                    cout << "Название: ";
                    cin >> software.name;

                    cout << "Версия (в формате X.Y.Z, пример (1.0.0)): ";
                    cin >> software.version;

                    if (!adminPanel.InstallSoftware(terminal.pid, software)) {
                        cout << "Установить ПО не удалось" << endl;
                        continue;
                    }
                    break;
                }
                case 23: {
                    cout << "Обновить ПО" << endl;

                    cout << "Выберите ПО которое хотите обновить" << endl;
                    vector<Software> software = adminPanel.GetSoftware(terminal.pid);
                    for (int i = 0; i < software.size(); ++i) {
                        cout << "[" << i + 1 << "]" << software.at(i).name << " (v " + software.at(i).version + ")"
                             << endl;
                    }

                    int num;
                    cin >> num;
                    if (num == -1) {
                        continue;
                    }

                    num -= 1;
                    if (num > software.size() || num < 0) {
                        cout << "Некорректные данные" << endl;
                        continue;
                    }

                    Software oldSoftware = software.at(num);
                    Software newSoftware = software.at(num);

                    cout << "Укажите новую версия (в формате X.Y.Z, пример (1.0.0)): ";
                    cin >> newSoftware.version;

                    if (!adminPanel.UpdateSoftware(terminal.pid, oldSoftware, newSoftware)) {
                        cout << "Не удалось обновить ПО" << endl;
                    }
                    break;
                }
                case 24: {
                    cout << "Удалить ПО" << endl;

                    cout << "Выберите ПО которое хотите удалить" << endl;
                    vector<Software> software = adminPanel.GetSoftware(terminal.pid);
                    for (int i = 0; i < software.size(); ++i) {
                        cout << "[" << i + 1 << "]" << software.at(i).name << " (v " + software.at(i).version + ")"
                             << endl;
                    }

                    int num;
                    cin >> num;
                    if (num == -1) {
                        continue;
                    }

                    num -= 1;
                    if (num > software.size() || num < 0) {
                        cout << "Некорректные данные" << endl;
                        continue;
                    }

                    if (!adminPanel.RemoveSoftware(terminal.pid, software.at(num))) {
                        cout << "Не удалось удалить ПО" << endl;
                    }
                    break;
                }
                case 31: {
                    cout << "Текущие пользователи:" << endl;
                    for (auto &user: adminPanel.GetUsers(terminal.pid)) {
                        cout << user.username << endl;
                    }
                    break;
                }
                case 32: {
                    cout << "Добавление нового пользователя" << endl;

                    User user;
                    cout << "Заполните данные" << endl;

                    cout << "Название: ";
                    cin >> user.username;

                    if (!adminPanel.AddUser(terminal.pid, user)) {
                        cout << "Добавить пользователя не удалось" << endl;
                        continue;
                    }
                    break;
                }
                case 33: {
                    cout << "Обновить права пользователя" << endl;

                    cout << "Выберите пользователя" << endl;
                    vector<User> users = adminPanel.GetUsers(terminal.pid);
                    for (int i = 0; i < users.size(); ++i) {
                        cout << "[" << i + 1 << "]" << users.at(i).username << endl;
                    }

                    int num;
                    cin >> num;
                    if (num == -1) {
                        continue;
                    }

                    num -= 1;
                    if (num > users.size() || num < 0) {
                        cout << "Некорректные данные" << endl;
                        continue;
                    }

                    User olduser = users.at(num);
                    User user = users.at(num);

                    bool innerExit = false;
                    while (!innerExit) {
                        cout << "Выберите дейстие:" << endl;
                        cout << "[1] Добавить права на ПО:" << endl;
                        cout << "[2] Удалить права на ПО:" << endl;
                        cout << "[3] Сохранить:" << endl;
                        cout << "[0] Выйти без сохранения:" << endl;
                        int userRightAction;
                        cin >> userRightAction;
                        switch (userRightAction) {
                            case 1: {
                                cout << "Выберите ПО права на которое хотите дать (-1 чтобы не выбирать):" << endl;
                                vector<Software> software = adminPanel.GetSoftware(terminal.pid);
                                for (int i = 0; i < software.size(); ++i) {
                                    cout << "[" << i + 1 << "]" << software.at(i).name
                                         << " (v " + software.at(i).version + ")" << endl;
                                }

                                int numInner;
                                cin >> numInner;
                                if (numInner == -1) {
                                    continue;
                                }

                                numInner -= 1;
                                if (numInner > software.size() || numInner < 0) {
                                    cout << "Некорректные данные" << endl;
                                    continue;
                                }

                                user.allowed_software.push_back(software.at(numInner));
                                break;
                            }
                            case 2: {
                                cout << "Текущие права (-1 чтобы не выбирать):" << endl;
                                vector<Software> software = user.allowed_software;
                                for (int i = 0; i < software.size(); ++i) {
                                    cout << "[" << i + 1 << "]" << software.at(i).name
                                         << " (v " + software.at(i).version + ")" << endl;
                                }

                                int numInner;
                                cin >> numInner;
                                if (numInner == -1) {
                                    continue;
                                }

                                numInner -= 1;
                                if (numInner > software.size() || numInner < 0) {
                                    cout << "Некорректные данные" << endl;
                                    continue;
                                }

                                user.allowed_software.erase(next(user.allowed_software.begin() + numInner - 1));
                                break;
                            }
                            case 3: {
                                if (!adminPanel.ChangeUserRights(terminal.pid, olduser, user)) {
                                    cout << "Не удалось обновить ПО" << endl;
                                }
                                innerExit = true;
                                break;
                            }
                            case 0: {
                                innerExit = true;
                                break;
                            }
                        }
                    }

                    break;
                }
                case 34: {
                    cout << "Удалить пользователя" << endl;

                    cout << "Выберите пользователя которого хотите удалить (-1 чтобы не выбирать)" << endl;
                    vector<User> users = adminPanel.GetUsers(terminal.pid);
                    for (int i = 0; i < users.size(); ++i) {
                        cout << "[" << i + 1 << "]" << users.at(i).username << endl;
                    }

                    int num;
                    cin >> num;
                    if (num == -1) {
                        continue;
                    }

                    num -= 1;
                    if (num > users.size() || num < 0) {
                        cout << "Некорректные данные" << endl;
                        continue;
                    }

                    if (!adminPanel.RemoveUser(terminal.pid, users.at(num))) {
                        cout << "Не удалось удалить пользователя" << endl;
                    }

                    break;
                }
                case 41: {
                    tuple<User, bool> tpl = adminPanel.GetLogginedUser(terminal.pid);
                    if (!get<1>(tpl)) {
                        cout << "Сейчас никто из пользователей не занял сеанс" << endl;
                        continue;
                    }
                    cout << "Пользователь текущего сеанса: " << get<0>(tpl).username << endl;
                    break;
                }
                case 0: {
                    adminPanel.ExitChoseTerminal();
                    break;
                }
            }
        } else {
            cout << "[-] Основное меню" << endl;
            cout << "[1] Состояние всех терминалов" << endl;
            cout << "[2] Выбрать терминал" << endl;
            cout << "[3] Отключить все терминалы" << endl;
            cout << "[4] Включить все терминалы" << endl;
            cout << "[0] СТОП" << endl;
            int action;
            cin >> action;
            switch (action) {
                case 1: {
                    cout << "Состояние всех терминалов" << endl;
                    for (auto &terminal: adminPanel.GetTerminals()) {
                        TerminalState state = adminPanel.GetTerminalState(terminal.pid);
                        cout << "Terminal (PID " << terminal.pid << ") state: " << TerminalStateToString(state)
                             << endl;
                    }
                    break;
                }
                case 2: {
                    cout << "[-] Выберите терминал (или -1 чтобы не выбирать)" << endl;
                    vector<Terminal> terminals = adminPanel.GetTerminals();
                    for (int i = 0; i < terminals.size(); i++) {
                        cout << "[" << i + 1 << "] " << "Terminal with PID: " << terminals.at(i).pid << endl;
                    }

                    int num;
                    cin >> num;
                    if (num == -1) {
                        continue;
                    }

                    num -= 1;
                    if (num > terminals.size() || num < 0) {
                        cout << "Некорректные данные" << endl;
                        continue;
                    }

                    adminPanel.ChoseTerminal(adminPanel.GetTerminals().at(num));
                    break;
                }
                case 3: {
                    for (auto &terminal: adminPanel.GetTerminals()) {
                        if (!adminPanel.SetTerminalState(terminal.pid, TerminalState::Offline)) {
                            cout << "Изменить состояние терминала с PID " << terminal.pid <<" не удалось" << endl;
                        }
                    }
                    break;
                }
                case 4: {
                    for (auto &terminal: adminPanel.GetTerminals()) {
                        if (!adminPanel.SetTerminalState(terminal.pid, TerminalState::Online)) {
                            cout << "Изменить состояние терминала с PID " << terminal.pid <<" не удалось" << endl;
                        }
                    }
                    break;
                }
                case 0: {
                    exit(0);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

int main() {
    // messages
    key_t key = ftok("/tmp", 's');
    int queue_id = msgget(key, IPC_CREAT | 0644);

    if (queue_id == -1) {
        std::cout << "Ошибка при создании очереди сообщений" << std::endl;
        return 1;
    }
    adminPanel = AdminPanel(queue_id);
    cout << "Подключение к очереди сообщений произошло успешно" << endl;


    cout << "Добавление удалённых терминалов" << endl;
    while (true) {
        cout << "Введите PID удалённого терминала (-1 чтобы продолжить): ";

        pid_t term_pid;
        cin >> term_pid;

        if (term_pid == -1) {
            break;
        }

        if (!adminPanel.AddTerminal(term_pid)) {
            cout << "Не удалось добавить терминал" << endl;
        }
    }

    menu();
}