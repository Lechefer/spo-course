#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include "lib.h"

#include <csignal>
#include <unistd.h>
#include <cstring>

#include "libs/nlohmann/json.h"

namespace nh = nlohmann;

class Terminal {
public:
    Terminal() {
        state = TerminalState::Offline;
        installed_software = vector<Software>();
        users = vector<User>();
        current_user = nullptr;
    };

    void On();

    TerminalState GetStatus();

    vector<Software> GetSoftware();

    bool InstallSoftware(Software software);

    bool UpdateSoftware(Software oldSoftware, Software software);

    bool RemoveSoftware(Software software);

    vector<User> GetUsers();

    bool AddUser(User user);

    bool ChangeUserRights(User olduser, User user);

    bool RemoveUser(User user);

    bool Login(const string &username);

    User *GetLogginedUser();

    void Logout();

    void Off();

private:
    TerminalState state;
    vector<Software> installed_software;
    vector<User> users;
    User *current_user;
};
