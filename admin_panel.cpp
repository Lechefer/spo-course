#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <csignal>
#include <unistd.h>
#include <cstring>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <cstring>
#include "lib.h"
#include "admin_panel.h"

using namespace std;

void send_message(int qid, long type, Request req) {
    struct Message msg;
    msg.mtype = type;
    std::memset(msg.mtext, '\0', sizeof(msg.mtext));

    nh::json j = req;
    msg.mtext_len = to_string(j).length();
    strcpy(msg.mtext, to_string(j).c_str());

    if ((msgsnd(qid, &msg, std::strlen(msg.mtext) + 1 + sizeof(msg.mtext_len), IPC_NOWAIT)) == -1) {
        perror("msgsnd");
        return;
    }
}

Response read_message(int qid, long type) {
    struct Message msg;
    msg.mtype = type;
    std::memset(msg.mtext, '\0', sizeof(msg.mtext));
    if (msgrcv(qid, (struct Message *) &msg, sizeof(Message), type, 0) == -1) {
        // perror("msgrcv");
        return Response{};
    }

    string str(msg.mtext, msg.mtext + msg.mtext_len);

    nh::json j = nh::json::parse(str);
    return j.get<Response>();
}

AdminPanel::AdminPanel(int queue_id) {
    msg_queue_id = queue_id;
    handshake_counter = 1;

    terminals = vector<Terminal>();
}

bool AdminPanel::AddTerminal(pid_t pid) {
    Terminal terminal{
            .pid = pid,
            .handshake_id = handshake_counter,
    };

    handshake_counter += 2;

    sigval_t value;
    value.sival_int = terminal.handshake_id;
    if (sigqueue(terminal.pid, SIGUSR1, value) == -1) {
        return false;
    }

    terminals.push_back(terminal);
    return true;
}

vector<Terminal> AdminPanel::GetTerminals() {
    return this->terminals;
}

Terminal AdminPanel::GetChosedTerminal() {
    return this->current_terminal;
}

bool AdminPanel::IsChosedTerminal() {
    return this->isChosedTerminal;
}

bool AdminPanel::ChoseTerminal(Terminal terminal) {
    if (this->isChosedTerminal) return false;

    this->current_terminal = terminal;
    this->isChosedTerminal = true;

    return true;
}

void AdminPanel::ExitChoseTerminal() {
    this->current_terminal = Terminal();
    this->isChosedTerminal = false;
}

TerminalState AdminPanel::GetTerminalState(pid_t pid) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "state";

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200) {
        return TerminalState::Undefined;
    }

    return response.state;
}

bool AdminPanel::SetTerminalState(pid_t pid, TerminalState state) {
    sigval_t value;

    switch (state) {
        case TerminalState::Online:
            value.sival_int = 1;
            break;
        case TerminalState::Offline:
            value.sival_int = 0;
            break;
        case TerminalState::Undefined:
            break;
    }

    return sigqueue(pid, SIGUSR2, value) != -1;
}

vector<Software> AdminPanel::GetSoftware(pid_t pid) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "software";

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200) {
        return vector<Software>();
    }

    return response.software;
}

bool AdminPanel::InstallSoftware(pid_t pid, Software software) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "install software";
    request.software = software;

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200) {
        return false;
    }

    return true;
}

bool AdminPanel::UpdateSoftware(pid_t pid, Software oldSoftware, Software software) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "update software";
    request.oldSoftware = oldSoftware;
    request.software = software;

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200) {
        return false;
    }

    return true;
}

bool AdminPanel::RemoveSoftware(pid_t pid, Software software) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "remove software";
    request.software = software;

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200) {
        return false;
    }

    return true;
}

vector<User> AdminPanel::GetUsers(pid_t pid) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "users";

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200) {
        return vector<User>();
    }

    return response.users;
}

bool AdminPanel::AddUser(pid_t pid, User user) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "add user";
    request.user = user;

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200) {
        return false;
    }

    return true;
}

bool AdminPanel::ChangeUserRights(pid_t pid, User oldUser, User user) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "change user rights";
    request.oldUser = oldUser;
    request.user = user;

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200) {
        return false;
    }

    return true;
}

bool AdminPanel::RemoveUser(pid_t pid, User user) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "remove user";
    request.user = user;

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200) {
        return false;
    }

    return true;
}

tuple<User, bool> AdminPanel::GetLogginedUser(pid_t pid) {
    Terminal terminal = getTerminalByPID(pid);

    Request request{};
    request.route = "current loggined user";

    send_message(this->msg_queue_id, terminal.handshake_id, request);

    Response response = read_message(this->msg_queue_id, terminal.handshake_id + 1);
    if (response.status != 200 || !response.is_logined) {
        return {User{}, false};
    }

    return {response.current_loggined_user, true};
}

Terminal AdminPanel::getTerminalByPID(pid_t pid) {
    Terminal terminal;

    for (auto &term: this->GetTerminals()) {
        if (term.pid != pid) continue;

        terminal = term;
    }

    return terminal;
}
