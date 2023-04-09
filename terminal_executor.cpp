#pragma once

#include <iostream>
#include <string>
#include <algorithm>
#include <csignal>
#include <cstring>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "lib.h"
#include "terminal.h"

using namespace std;

Terminal terminal;
int terminal_handshake_id = -1;

void *menu(void *_) {
    while (true) {
        if (terminal.GetStatus() == TerminalState::Offline) {
            continue;
        }

        if (terminal.GetLogginedUser() != nullptr) {
            cout << "[-] Терминал" << endl;
            cout << "[1] Установленные программы" << endl;
            cout << "[2] Доступные программы" << endl;
            cout << "[0] Выйти" << endl;
            int action;
            cin >> action;
            switch (action) {
                case 1:
                    for (const auto &sw: terminal.GetSoftware()) {
                        cout << sw.name << " (v " + sw.version + ")" << endl;
                    }
                    break;
                case 2:
                    for (const auto &sw: terminal.GetLogginedUser()->allowed_software) {
                        cout << sw.name << " (v " + sw.version + ")" << endl;
                    }
                    break;
                case 0:
                    terminal.Logout();
                    break;
                default:
                    break;
            }
        } else {
            cout << "[-] Вход" << endl;
            cout << "[1] Логин" << endl;
            cout << "[0] СТОП" << endl;
            int action;
            cin >> action;
            switch (action) {
                case 1: {
                    cout << ">Имя пользователя: ";
                    string username;
                    cin >> username;
                    if (terminal.Login(username)) {
                        cout << "Вы успешно авторизовались как " << username << endl;
                        continue;
                    }
                    cout << "Авторизоваться не удалось" << endl;
                    break;
                }
                case 0:
                    exit(0);
                    break;
                default:
                    break;
            }
        }
    }
}

void handshake(int id, siginfo_t *siginfo, void *context) {
    if (id != SIGUSR1) return;

    terminal_handshake_id = siginfo->si_value.sival_int;
}

void signal_set_state(int id, siginfo_t *siginfo, void *context) {
    if (id != SIGUSR2) return;

    switch (siginfo->si_value.sival_int) {
        case 1:
            terminal.On();
        case 0:
            terminal.Off();
    };
}

void register_signal_handlers() {
    // init sigaction
    struct sigaction act{};
    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_SIGINFO;

    // on
    act.sa_sigaction = &handshake;
    sigaction(SIGUSR1, &act, nullptr);

    // signal_set_state
    act.sa_sigaction = &signal_set_state;
    sigaction(SIGUSR2, &act, nullptr);
}


void send_message(int qid, long type, Response req)
{
    printf("Sending a message ...\n");
    struct Message qbuf;
    qbuf.mtype = type;
    nh::json j = req;
    strcpy(qbuf.mtext, to_string(j).c_str());
    if((msgsnd(qid, (struct Message *)&qbuf,strlen(qbuf.mtext)+1, 0)) == -1)
    {
        perror("msgsnd");
    }
}

Request read_message(int qid, long type)
{
    printf("Reading a message ...");
    struct Message qbuf;
    qbuf.mtype = type;
    if (msgrcv(qid, (struct Message *)&qbuf, MAX_SEND_SIZE, type, 0) == -1) {
        perror("msgrcv");
    }

    nh::json j = nh::json::parse(qbuf.mtext);
    return j.get<Request>();
}

void msg_handlers(int queue_id) {
    while (true) {
        Request request = read_message(queue_id, terminal_handshake_id);

        if (request.route == "state") {
            Response response{};
            response.state = terminal.GetStatus();

            send_message(queue_id, terminal_handshake_id + 1, response);
        } else if (request.route == "software") {
            Message msgResp{};
            msgResp.mtype = terminal_handshake_id + 1;

            Response response{};
            response.software = terminal.GetSoftware();
            msgsnd(queue_id, &response, sizeof(response) - sizeof(long), 0);
        } else if (request.route == "install software") {
            Message msgResp{};
            msgResp.mtype = terminal_handshake_id + 1;

            Response response{};
            if (!terminal.InstallSoftware(request.software)) {
                response.status = 400;
            }
            msgsnd(queue_id, &response, sizeof(response) - sizeof(long), 0);
        } else if (request.route == "update software") {
            Message msgResp{};
            msgResp.mtype = terminal_handshake_id + 1;

            Response response{};
            if (terminal.UpdateSoftware(request.oldSoftware, request.software)) {
                response.status = 400;
            }
            msgsnd(queue_id, &response, sizeof(response) - sizeof(long), 0);
        } else if (request.route == "remove software") {
            Message msgResp{};
            msgResp.mtype = terminal_handshake_id + 1;

            Response response{};
            if (terminal.RemoveSoftware(request.software)) {
                response.status = 400;
            }
            msgsnd(queue_id, &response, sizeof(response) - sizeof(long), 0);
        } else if (request.route == "users") {
            Message msgResp{};
            msgResp.mtype = terminal_handshake_id + 1;

            Response response{};
            response.users = terminal.GetUsers();
            msgsnd(queue_id, &response, sizeof(response) - sizeof(long), 0);
        } else if (request.route == "add user") {
            Message msgResp{};
            msgResp.mtype = terminal_handshake_id + 1;

            Response response{};
            if (terminal.AddUser(request.user)) {
                response.status = 400;
            }
            msgsnd(queue_id, &response, sizeof(response) - sizeof(long), 0);
        } else if (request.route == "change user rights") {
            Message msgResp{};
            msgResp.mtype = terminal_handshake_id + 1;

            Response response{};
            if (terminal.ChangeUserRights(request.oldUser, request.user)) {
                response.status = 400;
            }
            msgsnd(queue_id, &response, sizeof(response) - sizeof(long), 0);
        } else if (request.route == "remove user") {
            Message msgResp{};
            msgResp.mtype = terminal_handshake_id + 1;

            Response response{};
            if (terminal.RemoveUser(request.user)) {
                response.status = 400;
            }
            msgsnd(queue_id, &response, sizeof(response) - sizeof(long), 0);
        } else if (request.route == "current loggined user") {
            Message msgResp{};
            msgResp.mtype = terminal_handshake_id + 1;

            Response response{};
            response.current_loggined_user = terminal.GetLogginedUser();
            msgsnd(queue_id, &response, sizeof(response) - sizeof(long), 0);
        }
    }
}

int main() {
    cout << "Terminal PID: " << getpid() << endl;
    terminal = Terminal();

    register_signal_handlers();

    cout << "Ожидание рукопожатия с сервером..." << endl;
    while (terminal_handshake_id == -1) {
        sleep(1);
    }
    cout << "Рукопожатие совершено. Установленный идентификатор: " << terminal_handshake_id << endl;

    // messages
    key_t key = ftok("/tmp", 's');
    int queue_id = msgget(key, IPC_CREAT | 0666);

    if (queue_id == -1) {
        std::cout << "Ошибка при создании очереди сообщений" << std::endl;
        return 1;
    }
    cout << "Подключение к очереди сообщений произошло успешно" << endl;

//    pthread_t menu_thr;
//    pthread_create(&menu_thr, nullptr, menu, nullptr);

    msg_handlers(queue_id);
}