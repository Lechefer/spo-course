#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <csignal>
#include <unistd.h>
#include <cstring>

#include "lib.h"

#include "libs/nlohmann/json.h"

namespace nh = nlohmann;
using namespace std;

struct Terminal {
    pid_t pid;
    int handshake_id;
};

class AdminPanel {
public:
    AdminPanel(int queue_id);

    bool AddTerminal(pid_t pid);

    vector<Terminal> GetTerminals();

    Terminal GetChosedTerminal();

    bool IsChosedTerminal();

    bool ChoseTerminal(Terminal terminal);

    void ExitChoseTerminal();

    TerminalState GetTerminalState(pid_t pid);

    bool SetTerminalState(pid_t pid, TerminalState state);

    vector<Software> GetSoftware(pid_t pid);

    bool InstallSoftware(pid_t pid, Software software);

    bool UpdateSoftware(pid_t pid, Software oldSoftware, Software software);

    bool RemoveSoftware(pid_t pid, Software software);

    vector<User> GetUsers(pid_t pid);

    bool AddUser(pid_t pid, User user);

    bool ChangeUserRights(pid_t pid, User oldUser, User user);

    bool RemoveUser(pid_t pid, User user);

    User *GetLogginedUser(pid_t pid);

private:
    int msg_queue_id;
    int handshake_counter = 0;

    vector<Terminal> terminals;
    Terminal current_terminal;
    bool isChosedTerminal = false;

    Terminal getTerminalByPID(pid_t pid);
};
